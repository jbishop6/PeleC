#ifndef PROB_H
#define PROB_H

#include <AMReX_Print.H>
#include <AMReX_ParmParse.H>
#include <AMReX_Geometry.H>
#include <AMReX_FArrayBox.H>
#include <AMReX_REAL.H>
#include <AMReX_GpuMemory.H>

#include "mechanism.H"

#include "PeleC.H"
#include "IndexDefines.H"
#include "Constants.H"
#include "PelePhysics.H"
#include "Tagging.H"

#include "ProblemSpecificFunctions.H"
#include "prob_parm.H"

AMREX_GPU_DEVICE
AMREX_FORCE_INLINE
void
pc_initdata(
  int i,
  int j,
  int k,
  amrex::Array4<amrex::Real> const& state,
  amrex::GeometryData const& geomdata,
  ProbParmDevice const& prob_parm)
{

    // ============================================================
    // UNIFORM FRESH C2H4/O2/N2 MIXTURE
    //
    // Brown 1999 Shepherd validation condition:
    //   C2H4 + 3 O2 + 12 N2
    //
    // Thermodynamic state is specified through prob_parm.
    // ============================================================
  
    const amrex::Real rho = prob_parm.rho_r;
    const amrex::Real u   = prob_parm.u_r;
  
    // Density
    state(i, j, k, URHO) = rho;
  
    // Momentum
    state(i, j, k, UMX) = rho * u;
  
  #if AMREX_SPACEDIM >= 2
    state(i, j, k, UMY) = 0.0;
  #endif
  
  #if AMREX_SPACEDIM == 3
    state(i, j, k, UMZ) = 0.0;
  #endif
  
    // Internal energy
    state(i, j, k, UEINT) = prob_parm.rhoe_r;
  
    // Total energy
    state(i, j, k, UEDEN) =
      prob_parm.rhoe_r + 0.5 * rho * u * u;
  
    // Temperature
    state(i, j, k, UTEMP) = prob_parm.T_r;
  
    // Species densities
    for (int n = 0; n < NUM_SPECIES; ++n) {
      state(i, j, k, UFS + n) = rho * prob_parm.Y_r[n];
    }
  }
// ============================================================
// BOUNDARY CONDITION
// ============================================================

AMREX_GPU_DEVICE
AMREX_FORCE_INLINE
void
bcnormal(
  const amrex::Real* /*x[AMREX_SPACEDIM]*/,
  const amrex::Real s_int[NVAR],
  amrex::Real s_ext[NVAR],
  const int /*idir*/,
  const int /*sgn*/,
  const amrex::Real /*time*/,
  amrex::GeometryData const& /*geomdata*/,
  ProbParmDevice const& /*prob_parm*/,
  const amrex::GpuArray<amrex::Real, AMREX_SPACEDIM>& /*turb_fluc*/)
{
  for (int n = 0; n < NVAR; n++) {
    s_ext[n] = s_int[n];
  }
}


// ============================================================
// OPTIONAL PROBLEM CLOSE FUNCTION
// ============================================================

inline void
pc_prob_close()
{
}


// ============================================================
// DEFAULT PROBLEM-SPECIFIC FUNCTIONS
// ============================================================

using ProblemSpecificFunctions =
  DefaultProblemSpecificFunctions;

#endif
