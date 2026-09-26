#include "prob.H"

#include <AMReX_ParmParse.H>

using namespace amrex;

extern "C" {

void
amrex_probinit(
    const int*,
    const int*,
    const int*,
    const amrex::Real* problo,
    const amrex::Real* probhi)
{
    auto* P = PeleC::h_prob_parm_device;
    auto eos = pele::physics::PhysicsType::eos();

    // ------------------------------------------------------------
    // UNIFORM SHEPHERD VALIDATION STATE
    // Brown 1999: 1291 K, 3.31 atm
    // ------------------------------------------------------------
    
    P->T_r = 1291.0;          // K
    P->p_r = 3.3538575e6;     // dyn/cm^2 = 3.31 atm
    P->u_r = 0.0;             // cm/s

    // ============================================================
    // READ INPUT OVERRIDES
    // ============================================================

    amrex::ParmParse pp("prob");

    pp.query("T_r", P->T_r);
    pp.query("p_r", P->p_r);
    pp.query("u_r", P->u_r);

// ============================================================
// SHEPHERD / BROWN 1999 REACTANT COMPOSITION
//
// Mole ratio:
//   C2H4 + 3 O2 + 12 N2
//
// Converted to mass fractions for PeleC.
// ============================================================

amrex::Real Xr[NUM_SPECIES] = {0.0};
amrex::Real Yr[NUM_SPECIES] = {0.0};

// Exact Brown 1999 mole fractions:
// C2H4 + 3 O2 + 12 N2
Xr[C2H4_ID] = 1.0 / 16.0;
Xr[O2_ID]   = 3.0 / 16.0;
Xr[N2_ID]   = 12.0 / 16.0;

// Convert to mechanism-consistent mass fractions
eos.X2Y(Xr, Yr);

    // ============================================================
    // COMPUTE CONSISTENT RIGHT STATE
    // ============================================================

    amrex::Real e_r = 0.0;

    eos.PYT2R(
        P->p_r,
        Yr,
        P->T_r,
        P->rho_r);
    
    eos.RTY2E(
        P->rho_r,
        P->T_r,
        Yr,
        e_r);

    P->rhoe_r = P->rho_r * e_r;

    // ============================================================
    // DEBUG OUTPUT
    // ============================================================

   amrex::Print()
    << "\n========================================\n"
    << " SHEPHERD INDUCTION-TIME VALIDATION\n"
    << "========================================\n"
    << "T   = " << P->T_r << " K\n"
    << "p   = " << P->p_r << " dyn/cm^2\n"
    << "rho = " << P->rho_r << " g/cm^3\n"
    << "u   = " << P->u_r << " cm/s\n"
    << "\nREACTANT MIXTURE: C2H4 + 3 O2 + 12 N2\n"
    << "Y_C2H4 = " << Yr[C2H4_ID] << "\n"
    << "Y_O2   = " << Yr[O2_ID] << "\n"
    << "Y_N2   = " << Yr[N2_ID] << "\n"
    << "========================================\n\n";
}

} // extern "C"


void
PeleC::problem_post_init()
{
}

void
PeleC::problem_post_timestep()
{
}

void
PeleC::problem_post_restart()
{
}
