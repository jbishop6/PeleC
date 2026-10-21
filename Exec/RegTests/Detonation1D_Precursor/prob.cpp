
#include "prob.H"

#include <AMReX_ParmParse.H>
#include <AMReX_Print.H>
#include <AMReX.H>

using namespace amrex;

extern "C" {

void
amrex_probinit(
    const int*,
    const int*,
    const int*,
    const amrex::Real*,
    const amrex::Real*)
{
    auto* P = PeleC::h_prob_parm_device;
    auto eos = pele::physics::PhysicsType::eos();

    amrex::ParmParse pp("prob");

    // Fresh reactants: Cantera CJ reference state
    P->T_r = 300.0;
    P->p_r = 1.01325e6;
    P->u_r = 0.0;

    pp.query("T_r", P->T_r);
    pp.query("p_r", P->p_r);
    pp.query("u_r", P->u_r);

    // Initiation state: supplied through input file
    bool have_Tl = pp.query("T_l", P->T_l);
    bool have_pl = pp.query("p_l", P->p_l);
    pp.query("u_l", P->u_l);
    pp.query("x_interface", P->x_interface);

    AMREX_ALWAYS_ASSERT_WITH_MESSAGE(
        have_Tl && have_pl &&
        P->T_l > 0.0 && P->p_l > 0.0,
        "Specify positive prob.T_l and prob.p_l"
    );

    // C2H4 + 3 O2 + 12 N2
    amrex::Real X[NUM_SPECIES] = {0.0};
    amrex::Real Y[NUM_SPECIES] = {0.0};

    X[C2H4_ID] = 1.0 / 16.0;
    X[O2_ID]   = 3.0 / 16.0;
    X[N2_ID]   = 12.0 / 16.0;

    eos.X2Y(X, Y);

    
    // Right state: fresh ethylene-air reactants
    for (int n = 0; n < NUM_SPECIES; ++n) {
        P->Y_r[n] = Y[n];
    }

    // Left state: equilibrium CJ products from Cantera
    for (int n = 0; n < NUM_SPECIES; ++n) {
        P->Y_l[n] = 0.0;
    }

    P->Y_l[C2H4_ID] = 1.6366874174283230e-21;
    P->Y_l[H2_ID]   = 5.2256466613311513e-04;
    P->Y_l[O2_ID]   = 2.6250749996976387e-02;
    P->Y_l[H2O_ID]  = 7.3619680940778337e-02;
    P->Y_l[CO_ID]   = 3.8697656923326870e-02;
    P->Y_l[CO2_ID]  = 1.3045234470000630e-01;
    P->Y_l[N2_ID]   = 7.3045700272779030e-01;
    P->Y_l[HE_ID]   = 0.0;
    P->Y_l[AR_ID]   = 0.0;

    P->Y_l[H2_ID]  = 5.2256466613e-4;
    P->Y_l[O2_ID]  = 2.6250749997e-2;
    P->Y_l[H2O_ID] = 7.3619680941e-2;
    P->Y_l[CO_ID]  = 3.8697656923e-2;
    P->Y_l[CO2_ID] = 1.3045234470e-1;
    P->Y_l[N2_ID]  = 7.3045700277e-1;

    // Right-state thermodynamics
    amrex::Real e_r = 0.0;

    eos.PYT2R(P->p_r, P->Y_r,
              P->T_r, P->rho_r);

    eos.RTY2E(P->rho_r, P->T_r,
              P->Y_r, e_r);

    P->rhoe_r = P->rho_r * e_r;

    // Left-state thermodynamics
    amrex::Real e_l = 0.0;

    eos.PYT2R(P->p_l, P->Y_l,
              P->T_l, P->rho_l);

    eos.RTY2E(P->rho_l, P->T_l,
              P->Y_l, e_l);

    P->rhoe_l = P->rho_l * e_l;

    amrex::Print()
        << "\nDETONATION PROPAGATION INITIALIZATION\n"
        << "Right: T = " << P->T_r
        << " K, p = " << P->p_r << " dyn/cm^2\n"
        << "Left:  T = " << P->T_l
        << " K, p = " << P->p_l << " dyn/cm^2\n"
        << "Interface x = " << P->x_interface
        << " cm\n";
}

}

void PeleC::problem_post_init() {}
void PeleC::problem_post_timestep() {}
void PeleC::problem_post_restart() {}
