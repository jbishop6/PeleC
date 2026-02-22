#ifndef PROB_PARM_H
#define PROB_PARM_H

#include <AMReX_REAL.H>
#include <AMReX_GpuQualifiers.H>
#include <AMReX_GpuMemory.H>

struct ProbParmDevice {
  int idir;
  amrex::Real frac;

  amrex::Real rho_l, T_l, p_l, u_l, rhoe_l;
  amrex::Real rho_r, T_r, p_r, u_r, rhoe_r;

  int left_gas_id;
  int right_gas_id;
  amrex::Real split[AMREX_SPACEDIM];

  // NEW: premixed composition for left/right states
  amrex::Real Y_l[NUM_SPECIES];
  amrex::Real Y_r[NUM_SPECIES];
};

struct ProbParmHost
{
  std::string gasL = "N2";
  std::string gasR = "HE";
  ProbParmHost() = default;
};
#endif
