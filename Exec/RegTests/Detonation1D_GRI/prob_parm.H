#ifndef PROB_PARM_H
#define PROB_PARM_H

#include <AMReX_REAL.H>
#include <AMReX_GpuQualifiers.H>
#include <AMReX_GpuMemory.H>
#include "mechanism.H"

struct ProbParmDevice
{

  // Uniform fresh-reactant state
  amrex::Real p_r   = -1.0;
  amrex::Real u_r   = 0.0;
  amrex::Real rho_r = 0.0;
  amrex::Real rhoe_r = 0.0;
  amrex::Real T_r   = -1.0;

// Species mass fractions for uniform fresh-reactant state
amrex::Real Y_r[NUM_SPECIES] = {0.0};

};

struct ProbParmHost
{
  ProbParmHost() = default;
};

#endif
