#include "prob.H"
#include <AMReX_ParmParse.H>
#include <string>
#include "eb_geometry.H" // includes setupEBGeometry()
#include <AMReX_Print.H>
#include <AMReX_Box.H>
#include <AMReX_RealBox.H>
#include <AMReX_Geometry.H>
#include <vector>

using namespace amrex;

// Map species name -> index
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
void amrex_probinit(const int* /*init*/,
                    const int* /*name*/,
                    const int* /*namelen*/,
                    const amrex::Real* problo,
                    const amrex::Real* probhi)
{
    auto* P = PeleC::h_prob_parm_device;

    // Default values
    P->idir = 1;        P->frac = 0.5;
    P->rho_l = 9.6e-4;  P->rho_r = 1.2e-4;
    P->u_l = 0.0;       P->u_r = 0.0;
    P->T_l = 300.0;     P->T_r = 300.0;
    P->p_l = 1.0e5;     P->p_r = 1.0e5;

    amrex::ParmParse pp("prob");
    pp.query("idir",  P->idir);
    pp.query("frac",  P->frac);
    pp.query("rho_l", P->rho_l);
    pp.query("rho_r", P->rho_r);
    pp.query("u_l",   P->u_l);
    pp.query("u_r",   P->u_r);
    pp.query("T_l",   P->T_l);
    pp.query("T_r",   P->T_r);
    pp.query("p_l",   P->p_l);
    pp.query("p_r",   P->p_r);

    std::string leftGas = "N2", rightGas = "HE";
    pp.query("left_gas",  leftGas);
    pp.query("right_gas", rightGas);
    P->left_gas_id  = species_id_from_name(leftGas);
    P->right_gas_id = species_id_from_name(rightGas);

    for (int d = 0; d < AMREX_SPACEDIM; ++d) {
        P->split[d] = problo[d] + P->frac * (probhi[d] - problo[d]);
    }

    auto eos = pele::physics::PhysicsType::eos();
    amrex::Real e = 0.0;

    amrex::Real Yl[NUM_SPECIES] = {0.0};
    if (P->left_gas_id >= 0 && P->left_gas_id < NUM_SPECIES) Yl[P->left_gas_id] = 1.0;
    eos.RYP2E(P->rho_l, Yl, P->p_l, e);
    P->rhoe_l = P->rho_l * e;

    amrex::Real Yr[NUM_SPECIES] = {0.0};
    if (P->right_gas_id >= 0 && P->right_gas_id < NUM_SPECIES) Yr[P->right_gas_id] = 1.0;
    eos.RYP2E(P->rho_r, Yr, P->p_r, e);
    P->rhoe_r = P->rho_r * e;

    // Define geometry manually
    IntVect dom_lo(AMREX_D_DECL(0, 0, 0));
    IntVect dom_hi(AMREX_D_DECL(63, 0, 0));  // <- adjust for 1D domain size

    Box domain(dom_lo, dom_hi);
    RealBox real_box({AMREX_D_DECL(problo[0], problo[1], 0)},
                     {AMREX_D_DECL(probhi[0], probhi[1], 0)});

    int coord_sys = 0;
    Vector<int> is_periodic(AMREX_SPACEDIM, 0);
    Geometry geom(domain, &real_box, coord_sys, is_periodic.data());

    setupEBGeometry(geom, 0, 0);
}
}

void PeleC::problem_post_init() {}
void PeleC::problem_post_timestep() {}
void PeleC::problem_post_restart() {}
