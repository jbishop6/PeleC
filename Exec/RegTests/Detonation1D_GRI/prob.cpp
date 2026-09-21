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

Yl[H2_ID] = 5.3480812778242029e-04;
Yl[H_ID] = 9.0845292831370997e-05;
Yl[O_ID] = 1.4309714587010002e-03;
Yl[O2_ID] = 1.7265599219578147e-02;
Yl[OH_ID] = 7.2447248010601524e-03;
Yl[H2O_ID] = 7.2444816051833436e-02;
Yl[HO2_ID] = 1.3147377964560315e-05;
Yl[H2O2_ID] = 1.1016229856037651e-06;
Yl[C_ID] = 2.7856534556670474e-13;
Yl[CH_ID] = 5.6329387592078367e-14;
Yl[CH2_ID] = 6.9199631471188911e-14;
Yl[CH2_S_ID] = 6.7468672250847477e-15;
Yl[CH3_ID] = 1.2123152137080247e-13;
Yl[CH4_ID] = 2.3684104296454622e-14;
Yl[CO_ID] = 4.1132340714433648e-02;
Yl[CO2_ID] = 1.3539279495756548e-01;
Yl[HCO_ID] = 1.6193175763020338e-07;
Yl[CH2O_ID] = 2.4018587943377798e-09;
Yl[CH2OH_ID] = 1.6021855635344203e-13;
Yl[CH3O_ID] = 3.8855953727670352e-15;
Yl[CH3OH_ID] = 7.3198133595551782e-15;
Yl[C2H_ID] = 4.0721811763373890e-18;
Yl[C2H2_ID] = 8.1450834358975210e-17;
Yl[C2H3_ID] = 8.8122012429028440e-21;
Yl[C2H4_ID] = 2.3881010805285071e-21;
Yl[C2H5_ID] = 3.7333380918187557e-25;
Yl[C2H6_ID] = 1.2699094512330647e-26;
Yl[HCCO_ID] = 4.6418745637807222e-15;
Yl[CH2CO_ID] = 2.1435792852642753e-15;
Yl[HCCOH_ID] = 1.1875923542539838e-17;
Yl[N_ID] = 8.3758867019183763e-07;
Yl[NH_ID] = 1.8258274780188177e-07;
Yl[NH2_ID] = 5.3525352456782261e-08;
Yl[NH3_ID] = 4.4083683851715267e-08;
Yl[NNH_ID] = 1.1868412044278501e-07;
Yl[NO_ID] = 1.1891508602216661e-02;
Yl[NO2_ID] = 1.2722704870130638e-05;
Yl[N2O_ID] = 3.9823722079269673e-06;
Yl[HNO_ID] = 1.6929001013252661e-06;
Yl[CN_ID] = 2.0030582924378113e-10;
Yl[HCN_ID] = 7.1267437444159495e-09;
Yl[H2CN_ID] = 5.4844132134267437e-14;
Yl[HCNN_ID] = 2.4573691101263426e-16;
Yl[HCNO_ID] = 8.2222418611549181e-13;
Yl[HOCN_ID] = 7.6013891434121428e-10;
Yl[HNCO_ID] = 7.1636163515583306e-08;
Yl[NCO_ID] = 1.1962857620806264e-08;
Yl[N2_ID] = 7.1253745130985602e-01;
Yl[AR_ID] = 0.0000000000000000e+00;
Yl[C3H7_ID] = 3.8623651528174604e-37;
Yl[C3H8_ID] = 1.2050879940766907e-38;
Yl[CH2CHO_ID] = 1.3430700218427848e-19;
Yl[CH3CHO_ID] = 1.4224286444248996e-20;


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

// Store compositions for cell initialization in prob.H
for (int n = 0; n < NUM_SPECIES; ++n) {
    P->Y_l[n] = Yl[n];
    P->Y_r[n] = Yr[n];
}

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
