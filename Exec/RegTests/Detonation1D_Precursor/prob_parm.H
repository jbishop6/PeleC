#ifndef PROB_PARM_H
#define PROB_PARM_H

#include <AMReX_REAL.H>
#include <AMReX_GpuQualifiers.H>
#include <AMReX_GpuMemory.H>

struct ProbParmDevice
{
  // Left / ignition state
  amrex::Real p_l   = -1.0;
  amrex::Real u_l   = 0.0;
  amrex::Real rho_l = 0.0;
  amrex::Real rhoe_l = 0.0;
  amrex::Real T_l   = -1.0;

  // Right / fresh-reactant state
  amrex::Real p_r   = -1.0;
  amrex::Real u_r   = 0.0;
  amrex::Real rho_r = 0.0;
  amrex::Real rhoe_r = 0.0;
  amrex::Real T_r   = -1.0;

  // Fraction of domain occupied by the ignition/driver region
  amrex::Real frac = 0.05;

  // In 1-D this will be x-direction
  int idir = 0;

  amrex::GpuArray<amrex::Real, AMREX_SPACEDIM> split;
};

struct ProbParmHost
{
  ProbParmHost() = default;
};

#endif
