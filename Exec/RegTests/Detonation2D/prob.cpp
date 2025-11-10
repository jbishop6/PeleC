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

    // --- Hotspot geometry ---
    const amrex::Real hotspot_width = 0.003;   // 3 mm slab near x_lo

    // --- Base (fresh mixture) conditions ---
    P->rho_base = 1.2;           // kg/m^3  (≈ air density)
    P->T_base   = 300.0;         // K
    P->p_base   = 1.0e5;         // Pa

    // --- Hotspot (ignition) conditions ---
    P->T_hot = 3000.0;           // K
    P->p_hot = 1.0e5;            // same pressure (can raise to 5e5 if needed)

    // --- Mixture composition (stoich H2/air) ---
    amrex::Real Ymix[NUM_SPECIES] = {0.0};
    Ymix[species_id_from_name("H2")] = 0.028;
    Ymix[species_id_from_name("O2")] = 0.224;
    Ymix[species_id_from_name("N2")] = 0.748;  // 3.76× O2 fraction
    // normalize just in case
    amrex::Real sumY = 0.0;
    for (int n=0; n<NUM_SPECIES; ++n) sumY += Ymix[n];
    for (int n=0; n<NUM_SPECIES; ++n) Ymix[n] /= sumY;

    // store for device use if you have prob.H vars (optional)
    for (int n=0; n<NUM_SPECIES; ++n) P->Y_base[n] = Ymix[n];
    for (int n=0; n<NUM_SPECIES; ++n) P->Y_hot[n]  = Ymix[n];

    // compute rho*e for each state
    amrex::Real e;
    eos.RYP2E(P->rho_base, Ymix, P->p_base, e);
    P->rhoe_base = P->rho_base * e;
    eos.RYP2E(P->rho_base, Ymix, P->p_hot, e);   // same rho, higher T
    P->rhoe_hot = P->rho_base * e;

    // store hotspot spatial extent for later use in prob_initdata (on GPU)
    P->hotspot_xhi = problo[0] + hotspot_width;
}
} // extern "C"

// Optional hooks
void PeleC::problem_post_init() {}
void PeleC::problem_post_timestep() {}
void PeleC::problem_post_restart() {}
