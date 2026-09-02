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

    // ============================================================
    // DEFAULT 1-D DETONATION PRECURSOR PARAMETERS
    // ============================================================

    // Hot driver region occupies the left fraction of the domain
    P->idir = 0;
    P->frac = 0.05;

    // ------------------------------------------------------------
    // LEFT STATE: hot ignition / driver mixture
    // ------------------------------------------------------------

    P->T_l = 2500.0;     // K
    P->p_l = 5.0e5;      // Pa
    P->u_l = 0.0;        // m/s

    // ------------------------------------------------------------
    // RIGHT STATE: fresh premixed reactants
    // ------------------------------------------------------------

    P->T_r = 300.0;      // K
    P->p_r = 1.0e5;      // Pa
    P->u_r = 0.0;        // m/s

    // ============================================================
    // READ INPUT OVERRIDES
    // ============================================================

    amrex::ParmParse pp("prob");

    pp.query("idir", P->idir);
    pp.query("frac", P->frac);

    pp.query("T_l", P->T_l);
    pp.query("p_l", P->p_l);
    pp.query("u_l", P->u_l);

    pp.query("T_r", P->T_r);
    pp.query("p_r", P->p_r);
    pp.query("u_r", P->u_r);

    // ============================================================
    // VALIDATE 1-D DIRECTION
    // ============================================================

    if (P->idir != 0) {
        amrex::Abort("Detonation1D_Precursor requires prob.idir = 0");
    }

    // ============================================================
    // SPLIT LOCATION
    // ============================================================

    P->split[0] =
        problo[0] + P->frac * (probhi[0] - problo[0]);

// ============================================================
// LEFT STATE COMPOSITION: BURNED PRODUCTS
//
// Simple burned-product approximation for initial
// GRI-Mech 3.0 test: pure H2O
// ============================================================

amrex::Real Yl[NUM_SPECIES] = {0.0};

// GRI-Mech 3.0 equilibrium CJ products
Yl[H2_ID]   = 2.2592365184e-02;
Yl[H_ID]    = 5.6418811093e-03;
Yl[O_ID]    = 4.2570283138e-02;
Yl[O2_ID]   = 1.0694523501e-01;
Yl[OH_ID]   = 1.5779924332e-01;
Yl[H2O_ID]  = 6.6398235787e-01;
Yl[HO2_ID]  = 4.2107873568e-04;
Yl[H2O2_ID] = 4.7555139817e-05;


// ============================================================
// RIGHT STATE COMPOSITION: FRESH STOICHIOMETRIC H2/O2
//
// 2 H2 + O2
//
// mass fractions approximately:
// Y_H2 = 0.1119
// Y_O2 = 0.8881
// ============================================================

amrex::Real Yr[NUM_SPECIES] = {0.0};

Yr[H2_ID] = 0.1119;
Yr[O2_ID] = 0.8881;

    // ============================================================
    // COMPUTE CONSISTENT LEFT STATE
    // ============================================================

    amrex::Real e_l = 0.0;

    eos.PYT2R(
        P->p_l,
        Yl,
        P->T_l,
        P->rho_l);
    
    eos.RTY2E(
        P->rho_l,
        P->T_l,
        Yl,
        e_l);

    P->rhoe_l = P->rho_l * e_l;

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
        << " 1-D DETONATION PRECURSOR INITIALIZATION\n"
        << "========================================\n"
        << "split x = " << P->split[0] << "\n"
        << "\nLEFT BURNED PRODUCTS:\n"
        << "T   = " << P->T_l << "\n"
        << "p   = " << P->p_l << "\n"
        << "rho = " << P->rho_l << "\n"
        << "u   = " << P->u_l << "\n"
        << "\nRIGHT REACTANTS:\n"
        << "T   = " << P->T_r << "\n"
        << "p   = " << P->p_r << "\n"
        << "rho = " << P->rho_r << "\n"
        << "u   = " << P->u_r << "\n"
        << "\nLEFT PRODUCTS:\n"
        << "Y_H2O = " << Yl[H2O_ID] << "\n"
        
        << "\nRIGHT REACTANTS:\n"
        << "Y_H2 = " << Yr[H2_ID] << "\n"
        << "Y_O2 = " << Yr[O2_ID] << "\n"
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
