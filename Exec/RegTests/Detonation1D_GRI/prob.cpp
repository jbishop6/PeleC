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
    P->T_l = 2922.89536447;   // K
    P->p_l = 1.82039705e7;    // dyn/cm^2
    P->u_l = 8.17092650e4;    // cm/s

    // ------------------------------------------------------------
    // RIGHT STATE: fresh premixed reactants
    // ------------------------------------------------------------

    P->T_r = 300.0;      // K
    P->p_r = 1.0e6;   // dyn/cm^2 (equivalent to 100,000 Pa)
    P->u_r = 0.0;     // cm/s

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
// LEFT STATE COMPOSITION:
// BAURLE-EKLUND EQUILIBRIUM CJ PRODUCTS
// ============================================================

amrex::Real Yl[NUM_SPECIES] = {0.0};

Yl[H2_ID]  = 6.0552175215e-04;
Yl[O2_ID]  = 2.9936951915e-02;
Yl[H2O_ID] = 7.6466780497e-02;
Yl[CO_ID]  = 4.3988898172e-02;
Yl[CO2_ID] = 1.3090495130e-01;
Yl[N2_ID]  = 7.1809689637e-01;

// ============================================================
// RIGHT STATE COMPOSITION: FRESH C2H4/AIR
//
// Dr. Quinlan's supplied mixture:
// X_C2H4 = 0.065445
// X_O2   = 0.196340
// X_N2   = 0.738215
//
// Corresponding mass fractions:
// Y_C2H4 = 0.0637512142
// Y_O2   = 0.2181587710
// Y_N2   = 0.7180900148
// ============================================================

amrex::Real Yr[NUM_SPECIES] = {0.0};

Yr[C2H4_ID] = 0.0637512142;
Yr[O2_ID]   = 0.2181587710;
Yr[N2_ID]   = 0.7180900148;

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
        << "\nLEFT CJ PRODUCTS:\n"
        << "Y_H2  = " << Yl[H2_ID] << "\n"
        << "Y_O2  = " << Yl[O2_ID] << "\n"
        << "Y_H2O = " << Yl[H2O_ID] << "\n"
        << "Y_CO  = " << Yl[CO_ID] << "\n"
        << "Y_CO2 = " << Yl[CO2_ID] << "\n"
        << "Y_N2  = " << Yl[N2_ID] << "\n"
        
        << "\nRIGHT REACTANTS:\n"
        << "Y_C2H4 = " << Yr[C2H4_ID] << "\n"
        << "Y_O2   = " << Yr[O2_ID] << "\n"
        << "Y_N2   = " << Yr[N2_ID] << "\n"
        
        << "\nRIGHT REACTANTS:\n"
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
