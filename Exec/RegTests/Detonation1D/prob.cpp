// Exec/RegTests/Detonation1D/prob.cpp
#include "prob.H"
#include <AMReX_ParmParse.H>
#include <string>

using namespace amrex;

// Map species name -> index for this mechanism:
// H2 H AR N2 HE O OH HCO HO2 H2O CO O2 H2O2 CO2
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
    return 0; // fallback
}

extern "C" {
void amrex_probinit(const int* /*init*/,
                    const int* /*name*/,
                    const int* /*namelen*/,
                    const amrex::Real* problo,
                    const amrex::Real* probhi)
{
    auto* P = PeleC::h_prob_parm_device;

    // defaults, then override from inputs
    P->idir = 1;         P->frac = 0.5;
    P->rho_l = 9.6e-4;   P->rho_r = 1.2e-4;
    P->u_l = 0.0;        P->u_r = 0.0;
    P->T_l = 300.0;      P->T_r = 300.0;
    P->p_l = 1.0e5;      P->p_r = 1.0e5;

    ParmParse pp("prob");
    pp.query("idir",  P->idir);
    pp.query("frac",  P->frac);
    pp.query("rho_l", P->rho_l);
    pp.query("rho_r", P->rho_r);
    pp.query("u_l",   P->u_l);
    pp.query("u_r",   P->u_r);
    pp.query
