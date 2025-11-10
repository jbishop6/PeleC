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
    P->idir = 0;          // x-direction split
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

    // --- Gases ---
    std::string leftGas  = "H2";
    std::string rightGas = "O2";
    amrex::ParmParse pp("prob");
    pp.query("left_gas", leftGas);
    pp.query("right_gas", rightGas);

    P->left_gas_id  = species_id_from_name(leftGas);
    P->right_gas_id = species_id_from_name(rightGas);

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


// Optional hooks
void PeleC::problem_post_init() {}
void PeleC::problem_post_timestep() {}
void PeleC::problem_post_restart() {}
