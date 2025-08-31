#include "prob.H"
#include <AMReX_ParmParse.H>
#include <vector>

void
pc_prob_close()
{
}

extern "C" {
void
amrex_probinit(
  const int* /*init*/,
  const int* /*name*/,
  const int* /*namelen*/,
  const amrex::Real* problo,
  const amrex::Real* probhi)
{
  // Parse params
  {
    amrex::ParmParse pp("prob");
    pp.query("p_l", PeleC::h_prob_parm_device->p_l);
    pp.query("u_l", PeleC::h_prob_parm_device->u_l);
    pp.query("rho_l", PeleC::h_prob_parm_device->rho_l);
    pp.query("T_l", PeleC::h_prob_parm_device->T_l);
    pp.query("p_r", PeleC::h_prob_parm_device->p_r);
    pp.query("u_r", PeleC::h_prob_parm_device->u_r);
    pp.query("rho_r", PeleC::h_prob_parm_device->rho_r);
    pp.query("T_r", PeleC::h_prob_parm_device->T_r);
    pp.query("frac", PeleC::h_prob_parm_device->frac);
    pp.query("idir", PeleC::h_prob_parm_device->idir);
    pp.query("use_Tinit", PeleC::h_prob_parm_device->use_Tinit);

    std::string leftGas  = "N2", rightGas = "HE";
    pp.query("left_gas",  leftGas);
    pp.query("right_gas", rightGas);
    auto eos = pele::physics::PhysicsType::eos();
    PeleC::h_prob_parm_device->left_gas_id  = eos.speciesIndex(leftGas);
    PeleC::h_prob_parm_device->right_gas_id = eos.speciesIndex(rightGas);
  }
  
  for (int idir = 0; idir < AMREX_SPACEDIM; ++idir) {
    const amrex::Real lo = problo[idir];
    const amrex::Real hi = probhi[idir];
    PeleC::h_prob_parm_device->split[idir] =
        lo + PeleC::h_prob_parm_device->frac * (hi - lo);
  }

  amrex::Real e_l, e_r;
  auto eos = pele::physics::PhysicsType::eos();
  std::vector<amrex::Real> Y(eos.numSpecies(), 0.0);

   // Left state
  std::fill(Y.begin(), Y.end(), 0.0);
  if (PeleC::h_prob_parm_device->left_gas_id >= 0)
    Y[PeleC::h_prob_parm_device->left_gas_id] = 1.0;
  if (PeleC::h_prob_parm_device->use_Tinit) {
    eos.RTY2P(PeleC::h_prob_parm_device->rho_l, PeleC::h_prob_parm_device->T_l,
              Y.data(), PeleC::h_prob_parm_device->p_l);
  }
  eos.RYP2E(PeleC::h_prob_parm_device->rho_l, Y.data(),
            PeleC::h_prob_parm_device->p_l, e_l);
  PeleC::h_prob_parm_device->rhoe_l = PeleC::h_prob_parm_device->rho_l * e_l;

  // Right state
  std::fill(Y.begin(), Y.end(), 0.0);
  if (PeleC::h_prob_parm_device->right_gas_id >= 0)
    Y[PeleC::h_prob_parm_device->right_gas_id] = 1.0;
  if (PeleC::h_prob_parm_device->use_Tinit) {
    eos.RTY2P(PeleC::h_prob_parm_device->rho_r, PeleC::h_prob_parm_device->T_r,
              Y.data(), PeleC::h_prob_parm_device->p_r);
  }
  eos.RYP2E(PeleC::h_prob_parm_device->rho_r, Y.data(),
            PeleC::h_prob_parm_device->p_r, e_r);
  PeleC::h_prob_parm_device->rhoe_r = PeleC::h_prob_parm_device->rho_r * e_r;
}
} 
