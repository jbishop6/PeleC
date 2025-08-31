// Exec/RegTests/Detonation1D/prob.cpp
#include "prob.H"
#include <AMReX_ParmParse.H>
#include <vector>

using namespace amrex;

// ---- OPTIONAL (keep if prob.H doesn't inline it) ----
// void pc_prob_close() {}

extern "C"
void amrex_probinit(const int* /*init*/,
                    const int* /*name*/,
                    const amrex_real* problo,
                    const amrex_real* probhi)
{
  auto* P = PeleC::h_prob_parm_device;  // host-side problem params

  // defaults, then override from inputs
  P->idir = 1;        P->frac = 0.5;
  P->rho_l = 9.6e-4;  P->rho_r = 1.2e-4;
  P->u_l = 0.0;       P->u_r = 0.0;
  P->T_l = 300.0;     P->T_r = 300.0;
  P->p_l = 1.0e5;     P->p_r = 1.0e5;
  P->use_Tinit = 0;

  ParmParse pp("prob");
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
  pp.query("use_Tinit", P->use_Tinit);

  // Gas names -> species IDs used by prob.H
  std::string leftGas = "N2", rightGas = "HE";
  pp.query("left_gas",  leftGas);
  pp.query("right_gas", rightGas);

  auto eos = pele::physics::PhysicsType::eos();
  P->left_gas_id  = eos.speciesIndex(leftGas);
  P->right_gas_id = eos.speciesIndex(rightGas);

  // Correct split location: lo + frac*(hi - lo)
  for (int d = 0; d < AMREX_SPACEDIM; ++d) {
    P->split[d] = problo[d] + P->frac * (probhi[d] - problo[d]);
  }

  // Build rho*e on both sides from (rho, P, Y)
  std::vector<amrex::Real> Y(eos.numSpecies(), 0.0);
  amrex::Real e = 0.0;

  // Left
  std::fill(Y.begin(), Y.end(), 0.0);
  if (P->left_gas_id >= 0) Y[P->left_gas_id] = 1.0;
  if (P->use_Tinit) eos.RTY2P(P->rho_l, P->T_l, Y.data(), P->p_l);
  eos.RYP2E(P->rho_l, Y.data(), P->p_l, e);
  P->rhoe_l = P->rho_l * e;

  // Right
  std::fill(Y.begin(), Y.end(), 0.0);
  if (P->right_gas_id >= 0) Y[P->right_gas_id] = 1.0;
  if (P->use_Tinit) eos.RTY2P(P->rho_r, P->T_r, Y.data(), P->p_r);
  eos.RYP2E(P->rho_r, Y.data(), P->p_r, e);
  P->rhoe_r = P->rho_r * e;
}

// Optional hooks (fine as no-ops)
void PeleC::problem_post_init() {}
void PeleC::problem_post_timestep() {}
void PeleC::problem_post_restart() {}
