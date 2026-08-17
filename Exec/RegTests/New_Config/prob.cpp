// Exec/RegTests/Detonation1D/prob.cpp
#include "prob.H"
#include <AMReX_ParmParse.H>
#include <string>

using namespace amrex;

// Map species name -> index for your 14-spec mechanism
static int species_id_from_name(const std::string& s)
{
    if (s == "H2")   return 0;
    if (s == "H")    return 1;
    if (s == "AR")   return 2;
    if (s == "N2")   return 3;
    if (s == "HE")   return 4;
    if (s == "O")    return 5;
    if (s == "OH")   return 6;
    if (s == "HCO")  return 7;
    if (s == "HO2")  return 8;
    if (s == "H2O")  return 9;
    if (s == "CO")   return 10;
    if (s == "O2")   return 11;
    if (s == "H2O2") return 12;
    if (s == "CO2")  return 13;
    return 0;
}

extern "C" {
void amrex_probinit(const int*, const int*, const int*,
                    const amrex::Real* problo,
                    const amrex::Real* probhi)
{
    auto* P = PeleC::h_prob_parm_device;
    auto eos = pele::physics::PhysicsType::eos();

    // -------------------------------------------------
    //  ONE-SIDED DETONATION LAUNCH (hotspot on left)
    // -------------------------------------------------
    
    // DEFAULT VALUES (can be overridden by input file)
    P->idir = 0;          // x-direction split (default)
    P->frac = 0.02;       // hotspot occupies 2% of domain width

    // --- Left = hotspot ---
    P->rho_l = 1.2;        // kg/m^3
    P->T_l   = 3000.0;     // K (ignite)
    P->p_l   = 1.0e5;      // Pa (same as ambient or slightly higher)
    P->u_l   = 0.0;

    // --- Right = fresh mixture ---
    P->rho_r = 1.2;        // kg/m^3
    P->T_r   = 300.0;      // K
    P->p_r   = 1.0e5;      // Pa
    P->u_r   = 0.0;

    // --- Read from input file (OVERRIDE defaults) ---
    amrex::ParmParse pp("prob");
    
    pp.query("idir", P->idir);        // ADD THIS LINE
    pp.query("frac", P->frac);        // ADD THIS LINE
    pp.query("rho_l", P->rho_l);      // ADD THIS LINE
    pp.query("T_l", P->T_l);          // ADD THIS LINE
    pp.query("p_l", P->p_l);          // ADD THIS LINE
    pp.query("u_l", P->u_l);          // ADD THIS LINE
    pp.query("rho_r", P->rho_r);      // ADD THIS LINE
    pp.query("T_r", P->T_r);          // ADD THIS LINE
    pp.query("p_r", P->p_r);          // ADD THIS LINE
    pp.query("u_r", P->u_r);          // ADD THIS LINE

    // --- Gases ---
    std::string leftGas  = "H2";
    std::string rightGas = "O2";
    pp.query("left_gas", leftGas);
    pp.query("right_gas", rightGas);

    P->left_gas_id  = species_id_from_name(leftGas);
    P->right_gas_id = species_id_from_name(rightGas);

    // --- Validate idir ---
    if (P->idir < 0 || P->idir >= AMREX_SPACEDIM) {
        amrex::Abort("invalid idir !!!");
    }

    // --- Split location ---
    for (int d = 0; d < AMREX_SPACEDIM; ++d)
        P->split[d] = problo[d] + P->frac * (probhi[d] - problo[d]);

    // --- Compute rho*e for both states ---
    amrex::Real e = 0.0;
    amrex::Real Yl[NUM_SPECIES] = {0.0};
    if (P->left_gas_id >= 0 && P->left_gas_id < NUM_SPECIES)
        Yl[P->left_gas_id] = 1.0;
    eos.RYP2E(P->rho_l, Yl, P->p_l, e);
    P->rhoe_l = P->rho_l * e;

    amrex::Real Yr[NUM_SPECIES] = {0.0};
    if (P->right_gas_id >= 0 && P->right_gas_id < NUM_SPECIES)
        Yr[P->right_gas_id] = 1.0;
    eos.RYP2E(P->rho_r, Yr, P->p_r, e);
    P->rhoe_r = P->rho_r * e;
}
} // extern "C"

AMREX_GPU_HOST_DEVICE AMREX_FORCE_INLINE
void get_interpolated_data(
  const amrex::Array4<const amrex::Real>& arr,
  const amrex::RealVect& pos,
  const amrex::GeometryData& geom,
  int ncomp,
  amrex::Real* interp_state,
  amrex::Real& interp_pres)
{
    // Map physical position to cell-center-relative coordinates (2D)
    amrex::RealVect loc = (pos - amrex::RealVect(geom.ProbLo())) / amrex::RealVect(geom.CellSize()) - 0.5;
    amrex::IntVect ij(AMREX_D_DECL(static_cast<int>(std::floor(loc[0])),
                                  static_cast<int>(std::floor(loc[1])),
                                  0));
    amrex::RealVect frac = loc - amrex::RealVect(ij);

    interp_pres = 0.0;
    for (int n = 0; n < ncomp; ++n) interp_state[n] = 0.0;

    // Bilinear interpolation (4 nodes in 2D)
    for (int j = 0; j <= 1; ++j) {
        for (int i = 0; i <= 1; ++i) {
            amrex::IntVect cur_ij(ij[0] + i, ij[1] + j);
            amrex::Real weight = (i ? frac[0] : 1.0 - frac[0]) *
                                 (j ? frac[1] : 1.0 - frac[1]);

            // 1. Interpolate State Components
            for (int n = 0; n < ncomp; ++n) {
                interp_state[n] += weight * arr(cur_ij, n);
            }

            // 2. Local Pressure via EOS
            amrex::Real rho = arr(cur_ij, URHO);
            // Kinetic energy in 2D: 0.5 * (rhou^2 + rhov^2) / rho
            amrex::Real kin_eng = 0.5 * (arr(cur_ij, UMX)*arr(cur_ij, UMX) + 
                                         arr(cur_ij, UMY)*arr(cur_ij, UMY)) / rho;
            amrex::Real rho_e = arr(cur_ij, UEDEN) - kin_eng;
            
            amrex::Real mass_frac[NUM_SPECIES];
            for (int n = 0; n < NUM_SPECIES; ++n) mass_frac[n] = arr(cur_ij, UFS+n) / rho;
            
            amrex::Real p_local, T_local = 300.0, T;
            auto eos = pele::physics::PhysicsType::eos();
            // eos.REY2T(rho, rho_e/rho, mass_frac, T);
            // eos.RTY2P(rho, T, mass_frac, p_local);
            eos.RTY2P(rho, T_local, mass_frac, p_local);
            interp_pres += weight * p_local;
        }
    }
}


// void PeleC::problem_post_timestep(Amr* amrptr)
void PeleC::problem_post_timestep()
{
        // ----------------------------------------
    // Geometry-based 3-probe placement
    // P0: upstream main-channel centerline
    // P1: downstream main-channel centerline
    // P2: second outlet probe for pressure comparison
    // ----------------------------------------
    amrex::ParmParse pp("geo");

    amrex::Real W = 0.04, H = 0.04, L = 0.04, xs = 0.30;
    amrex::Real X = 0.40;
    amrex::Real mid = 0.02;
    amrex::Real cL  = 0.00;
    amrex::Real cR  = 0.00;
    amrex::Real y_offset = 0.0;
    amrex::Real Z = 0.08;

    pp.query("W", W);
    pp.query("H", H);
    pp.query("L", L);
    pp.query("xs", xs);
    pp.query("X", X);
    pp.query("mid", mid);
    pp.query("cL", cL);
    pp.query("cR", cR);
    pp.query("y_offset", y_offset);
    pp.query("Z", Z);

    const amrex::Geometry& geom0 = parent->Geom(0);

    const amrex::Real* problo = geom0.ProbLo();
    const amrex::Real* probhi = geom0.ProbHi();

    const amrex::Real xlo = problo[0];
    const amrex::Real xhi = probhi[0];
    const amrex::Real ylo = problo[1];
    const amrex::Real yhi = probhi[1];

    const amrex::Real dx = geom0.CellSize(0);
    const amrex::Real dy = geom0.CellSize(1);
    const amrex::Real h  = std::max(dx, dy);

    const amrex::Real ymid = 0.5 * (ylo + yhi) + y_offset;
    const amrex::Real xr   = xs + X;
    const amrex::Real mw_x1 = xr - cR;
    const amrex::Real z_x_right = mw_x1 + W;

    // Probe locations:
    // P0 = upstream main centerline
    // P1 = downstream main centerline
    // P2 = farther outlet probe for outlet pressure comparison
    amrex::Real x_p0 = xs - 6.0*h;
    amrex::Real x_p1 = z_x_right + 6.0*h;
    amrex::Real x_p2 = z_x_right + 12.0*h;

    // Keep probes safely inside the domain
    x_p0 = std::max(x_p0, xlo + 6.0*h);
    x_p1 = std::min(x_p1, xhi - 12.0*h);
    x_p2 = std::min(x_p2, xhi - 6.0*h);

    const int n_probes = 3;
    amrex::Vector<amrex::RealVect> probes = {
        amrex::RealVect(AMREX_D_DECL(x_p0, ymid, 0.0)),
        amrex::RealVect(AMREX_D_DECL(x_p1, ymid, 0.0)),
        amrex::RealVect(AMREX_D_DECL(x_p2, ymid, 0.0))
    };

    if (amrex::ParallelDescriptor::IOProcessor() && parent->levelSteps(0) == 0) {
        amrex::Print() << "\n[Probe setup]\n";
        amrex::Print() << "  P0 (wave-speed upstream) = (" << x_p0 << ", " << ymid << ")\n";
        amrex::Print() << "  P1 (wave-speed downstream / outlet ref) = (" << x_p1 << ", " << ymid << ")\n";
        amrex::Print() << "  P2 (outlet comparison) = (" << x_p2 << ", " << ymid << ")\n\n";
    }
    
    int ncomp = parent->getLevel(0).get_new_data(State_Type).nComp();
    int data_per_probe = ncomp + 1;
    amrex::Vector<amrex::Real> global_data(n_probes * data_per_probe, 0.0);

    for (int p = 0; p < n_probes; ++p) {
        bool found_locally = false;
        amrex::Vector<amrex::Real> local_combined(data_per_probe, 0.0);

        for (int lev = parent->finestLevel(); lev >= 0; --lev) {
            amrex::AmrLevel& amr_level = parent->getLevel(lev);
            const amrex::MultiFab& S_new = amr_level.get_new_data(State_Type);
            const auto geom_data = amr_level.Geom().data();
            amrex::IntVect iv = amr_level.Geom().CellIndex(probes[p].dataPtr());


            for (amrex::MFIter mfi(S_new); mfi.isValid(); ++mfi) {
                if (mfi.validbox().contains(iv)) {
                    get_interpolated_data(S_new.array(mfi), probes[p], geom_data, ncomp, 
                                          local_combined.data(), local_combined[ncomp]);
                    found_locally = true;
                    break;
                }
            }
            
            int global_found = found_locally ? 1 : 0;
            amrex::ParallelDescriptor::ReduceIntMax(global_found);

            if (global_found > 0) {
                amrex::ParallelDescriptor::ReduceRealSum(local_combined.data(), data_per_probe);
                for (int d = 0; d < data_per_probe; ++d) {
                    global_data[p * data_per_probe + d] = local_combined[d];
                }
                break; 
            }
        }
    }

    if (amrex::ParallelDescriptor::IOProcessor()) {
        const std::string filename = "probe_data_2d.dat";
        
        // 1. Check if the file is empty to handle restarts or fresh runs
        std::ifstream infile(filename);
        bool is_empty = infile.peek() == std::ifstream::traits_type::eof();
        infile.close();

        std::ofstream ofs(filename, std::ios::app);
        
        // 2. Write header if file is empty OR it's the very first step
        if (is_empty || parent->levelSteps(0) == 0) {
            ofs << "# 1:Time";
            int col = 2;
            
            // Get the descriptor list from level 0
            const auto& desc_lst = parent->getLevel(0).get_desc_lst();

            for (int p = 0; p < n_probes; ++p) {
                for (int n = 0; n < ncomp; ++n) {
                    // Retrieves the component name (e.g., density, x_mom)
                    ofs << " " << col++ << ":P" << p << "_" << desc_lst[State_Type].name(n);
                }
                ofs << " " << col++ << ":P" << p << "_Pressure";
            }
            ofs << "\n";
        }

        // 3. Write your data
        ofs << std::setprecision(12) << parent->cumTime();
        for (const auto& val : global_data) {
            ofs << " " << std::setprecision(8) << val;
        }
        ofs << "\n";
        ofs.close();
    }
}


// Optional hooks
void PeleC::problem_post_init() {}
// void PeleC::problem_post_timestep() {}
void PeleC::problem_post_restart() {}
