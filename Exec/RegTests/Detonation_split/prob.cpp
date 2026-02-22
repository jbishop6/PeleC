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
    // --- Build a premixed H2/O2 composition (stoichiometric 2H2 + O2) ---

amrex::Real Y_mix[NUM_SPECIES] = {0.0};

// Get indices for H2 and O2 from the mechanism
int iH2 = species_id_from_name("H2");
int iO2 = species_id_from_name("O2");

// Stoichiometric: 2 H2 (2 g/mol) + 1 O2 (32 g/mol)
// Total mass = 4 + 32 = 36
const amrex::Real m_H2 = 4.0;
const amrex::Real m_O2 = 32.0;
const amrex::Real m_tot = m_H2 + m_O2;

Y_mix[iH2] = m_H2 / m_tot;
Y_mix[iO2] = m_O2 / m_tot;

// Store mixture in prob parameters for left and right states
for (int n = 0; n < NUM_SPECIES; ++n) {
    P->Y_l[n] = Y_mix[n];
    P->Y_r[n] = Y_mix[n];
}

// Compute rho*e for left/right using this SAME mixture
amrex::Real e = 0.0;
eos.RYP2E(P->rho_l, Y_mix, P->p_l, e);
P->rhoe_l = P->rho_l * e;

eos.RYP2E(P->rho_r, Y_mix, P->p_r, e);
P->rhoe_r = P->rho_r * e;
}
} // extern "C"


// Optional hooks
void PeleC::problem_post_init() {}
void PeleC::problem_post_timestep() {}
void PeleC::problem_post_restart() {}
