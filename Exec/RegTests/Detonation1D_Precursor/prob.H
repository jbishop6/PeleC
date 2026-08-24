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
  // 1-D POSITION
  // ============================================================

  const amrex::Real* prob_lo = geomdata.ProbLo();
  const amrex::Real* dx = geomdata.CellSize();

  const amrex::Real x =
    prob_lo[0] + (i + 0.5) * dx[0];

  // ============================================================
  // STOICHIOMETRIC H2/O2 MIXTURE
  //
  // Reaction:
  //     2 H2 + O2
  //
  // Approximate mass fractions:
  //     Y_H2 = 0.1119
  //     Y_O2 = 0.8881
  //
  // Both sides use the SAME premixed reactants.
  // The left/right difference is thermodynamic.
  // ============================================================

  constexpr amrex::Real Y_H2 = 0.1119;
  constexpr amrex::Real Y_O2 = 0.8881;

  // Initialize all species to zero
  for (int n = 0; n < NUM_SPECIES; ++n) {
    state(i, j, k, UFS + n) = 0.0;
  }

  // ============================================================
  // LEFT STATE:
  // HOT DRIVER / IGNITION REGION
  // ============================================================

  if (x <= prob_parm.split[0]) {

    const amrex::Real rho = prob_parm.rho_l;
    const amrex::Real u   = prob_parm.u_l;

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
    state(i, j, k, UEINT) =
      prob_parm.rhoe_l;

    // Total energy = internal + kinetic
    state(i, j, k, UEDEN) =
      prob_parm.rhoe_l +
      0.5 * rho * u * u;

    // Temperature
    state(i, j, k, UTEMP) =
      prob_parm.T_l;

    // Species densities: rho * Y_k
    state(i, j, k, UFS + H2_ID) =
      rho * Y_H2;

    state(i, j, k, UFS + O2_ID) =
      rho * Y_O2;
  }

  // ============================================================
  // RIGHT STATE:
  // COLD FRESH REACTANTS
  // ============================================================

  else {

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
    state(i, j, k, UEINT) =
      prob_parm.rhoe_r;

    // Total energy = internal + kinetic
    state(i, j, k, UEDEN) =
      prob_parm.rhoe_r +
      0.5 * rho * u * u;

    // Temperature
    state(i, j, k, UTEMP) =
      prob_parm.T_r;

    // Same premixed H2/O2 composition
    state(i, j, k, UFS + H2_ID) =
      rho * Y_H2;

    state(i, j, k, UFS + O2_ID) =
      rho * Y_O2;
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
