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
pmf(
  amrex::Real xlo,
  amrex::Real xhi,
  amrex::GpuArray<amrex::Real, NUM_SPECIES + 4>& y_vector,
  const ProbParmDevice& prob_parm)
{
  if (prob_parm.pmf_do_average) {
    int lo_loside = 0;
    int lo_hiside = 0;
    int hi_loside = 0;
    int hi_hiside = 0;
    if (xlo < prob_parm.d_pmf_X[0]) {
      lo_loside = 0;
      lo_hiside = 0;
    }
    if (xhi < prob_parm.d_pmf_X[0]) {
      hi_loside = 0;
      hi_hiside = 0;
    }
    if (xlo > prob_parm.d_pmf_X[prob_parm.pmf_N - 1]) {
      lo_loside = prob_parm.pmf_N - 1;
      lo_hiside = prob_parm.pmf_N - 1;
    }
    if (xhi > prob_parm.d_pmf_X[prob_parm.pmf_N - 1]) {
      hi_loside = prob_parm.pmf_N - 1;
      hi_hiside = prob_parm.pmf_N - 1;
    }
    if (lo_loside == 0) {
      for (unsigned int i = 0; i < prob_parm.pmf_N - 1; i++) {
        if ((xlo > prob_parm.d_pmf_X[i]) && (xlo < prob_parm.d_pmf_X[i + 1])) {
          lo_loside = i;
          lo_hiside = i + 1;
        }
      }
    }
    if (hi_loside == 0) {
      for (unsigned int i = 0; i < prob_parm.pmf_N - 1; i++) {
        if ((xhi > prob_parm.d_pmf_X[i]) && (xhi < prob_parm.d_pmf_X[i + 1])) {
          hi_loside = i;
          hi_hiside = i + 1;
        }
      }
    }
    for (unsigned int j = 0; j < prob_parm.pmf_M; j++) {
      amrex::Real x1 = prob_parm.d_pmf_X[lo_loside];
      amrex::Real y1 = prob_parm.d_pmf_Y[prob_parm.pmf_N * j + lo_loside];
      amrex::Real x2 = prob_parm.d_pmf_X[lo_hiside];
      amrex::Real y2 = prob_parm.d_pmf_Y[prob_parm.pmf_N * j + lo_hiside];
      amrex::Real dydx = 0.0;
      if (lo_loside == lo_hiside)
        dydx = 0.0;
      else
        dydx = (y2 - y1) / (x2 - x1);
      amrex::Real ylo = y1 + dydx * (xlo - x1);
      amrex::Real yhi = 0.0;
      if (lo_loside == hi_loside) {
        yhi = y1 + dydx * (xhi - x1);
        y_vector[j] = 0.5 * (ylo + yhi);
      } else {
        amrex::Real sum = (x2 - xlo) * 0.5 * (ylo + y2);
        x1 = prob_parm.d_pmf_X[hi_loside];
        y1 = prob_parm.d_pmf_Y[prob_parm.pmf_N * j + hi_loside];
        x2 = prob_parm.d_pmf_X[hi_hiside];
        y2 = prob_parm.d_pmf_Y[prob_parm.pmf_N * j + hi_hiside];
        if (hi_loside == hi_hiside)
          dydx = 0.0;
        else
          dydx = (y2 - y1) / (x2 - x1);
        yhi = y1 + dydx * (xhi - x1);
        sum += (xhi - x1) * 0.5 * (yhi + y1);
        for (int k = lo_hiside; k < hi_loside - 1; k++) {
          sum += (prob_parm.d_pmf_X[k + 1] - prob_parm.d_pmf_X[k]) * 0.5 *
                 (prob_parm.d_pmf_Y[prob_parm.pmf_N * j + k] +
                  prob_parm.d_pmf_Y[prob_parm.pmf_N * j + k + 1]);
        }
        y_vector[j] = sum / (xhi - xlo);
      }
    }
  } else {
    amrex::Real xmid = 0.5 * (xlo + xhi);
    int loside = -1;
    int hiside = -1;
    if (xmid < prob_parm.d_pmf_X[0]) {
      loside = 0;
      hiside = 0;
    }
    if (xmid > prob_parm.d_pmf_X[prob_parm.pmf_N - 1]) {
      loside = prob_parm.pmf_N - 1;
      hiside = prob_parm.pmf_N - 1;
    }
    if (loside == -1) {
      for (unsigned int i = 0; i < prob_parm.pmf_N - 1; i++) {
        if (
          (xmid >= prob_parm.d_pmf_X[i]) &&
          (xmid <= prob_parm.d_pmf_X[i + 1])) {
          loside = i;
          hiside = i + 1;
        }
      }
    }
    for (unsigned int j = 0; j < prob_parm.pmf_M; j++) {
      const amrex::Real x1 = prob_parm.d_pmf_X[loside];
      const amrex::Real y1 = prob_parm.d_pmf_Y[prob_parm.pmf_N * j + loside];
      const amrex::Real x2 = prob_parm.d_pmf_X[hiside];
      const amrex::Real y2 = prob_parm.d_pmf_Y[prob_parm.pmf_N * j + hiside];
      amrex::Real dydx = 0.0;
      if (loside == hiside)
        dydx = 0.0;
      else
        dydx = (y2 - y1) / (x2 - x1);
      y_vector[j] = y1 + dydx * (xlo - x1);
    }
  }
}

// void
// pc_initdata(
//   int i,
//   int j,
//   int k,
//   amrex::Array4<amrex::Real> const& state,
//   amrex::GeometryData const& geomdata,
//   ProbParmDevice const& prob_parm)
// {
//   // Geometry
//   const amrex::Real* prob_lo = geomdata.ProbLo();
//   const amrex::Real* dx = geomdata.CellSize();
//   const amrex::Real x = prob_lo[0] + (i + 0.5) * dx[0];
//   const amrex::Real y = prob_lo[1] + (j + 0.5) * dx[1];
//   amrex::Real z = 0;
// #if AMREX_SPACEDIM == 3
//   z = prob_lo[2] + (k + 0.5) * dx[2];
// #endif
//   for (int n = 0; n < NUM_SPECIES; n++) {
//     state(i, j, k, UFS + n) = 0;
//   }
// 
//   // Set the states
//   if (prob_parm.idir == 1) {
//     if (x <= prob_parm.split[0]) {
//       state(i, j, k, URHO) = prob_parm.rho_l;
//       state(i, j, k, UMX) = prob_parm.rho_l * prob_parm.u_l;
//       state(i, j, k, UMY) = 0.0;
//       state(i, j, k, UMZ) = 0.0;
//       state(i, j, k, UEDEN) = prob_parm.rhoe_l + 0.5 * prob_parm.rho_l *
//                                                    prob_parm.u_l *
//                                                    prob_parm.u_l;
//       state(i, j, k, UEINT) = prob_parm.rhoe_l;
//       state(i, j, k, UTEMP) = prob_parm.T_l;
//       state(i, j, k, UFS + prob_parm.left_gas_id) = state(i, j, k, URHO);
//     } else {
//       state(i, j, k, URHO) = prob_parm.rho_r;
//       state(i, j, k, UMX) = prob_parm.rho_r * prob_parm.u_r;
//       state(i, j, k, UMY) = 0.0;
//       state(i, j, k, UMZ) = 0.0;
//       state(i, j, k, UEDEN) = prob_parm.rhoe_r + 0.5 * prob_parm.rho_r *
//                                                    prob_parm.u_r *
//                                                    prob_parm.u_r;
//       state(i, j, k, UEINT) = prob_parm.rhoe_r;
//       state(i, j, k, UTEMP) = prob_parm.T_r;
//       state(i, j, k, UFS + prob_parm.right_gas_id) = state(i, j, k, URHO);
//     }
//   } else if (prob_parm.idir == 2) {
//     if (y <= prob_parm.split[1]) {
//       state(i, j, k, URHO) = prob_parm.rho_l;
//       state(i, j, k, UMX) = 0.0;
//       state(i, j, k, UMY) = prob_parm.rho_l * prob_parm.u_l;
//       state(i, j, k, UMZ) = 0.0;
//       state(i, j, k, UEDEN) = prob_parm.rhoe_l + 0.5 * prob_parm.rho_l *
//                                                    prob_parm.u_l *
//                                                    prob_parm.u_l;
//       state(i, j, k, UEINT) = prob_parm.rhoe_l;
//       state(i, j, k, UTEMP) = prob_parm.T_l;
//       state(i, j, k, UFS + prob_parm.left_gas_id) = state(i, j, k, URHO);
//     } else {
//       state(i, j, k, URHO) = prob_parm.rho_r;
//       state(i, j, k, UMX) = 0.0;
//       state(i, j, k, UMY) = prob_parm.rho_r * prob_parm.u_r;
//       state(i, j, k, UMZ) = 0.0;
//       state(i, j, k, UEDEN) = prob_parm.rhoe_r + 0.5 * prob_parm.rho_r *
//                                                    prob_parm.u_r *
//                                                    prob_parm.u_r;
//       state(i, j, k, UEINT) = prob_parm.rhoe_r;
//       state(i, j, k, UTEMP) = prob_parm.T_r;
//       state(i, j, k, UFS + prob_parm.right_gas_id) = state(i, j, k, URHO);
//     }
//   } else if (prob_parm.idir == 3) {
//     if (z <= prob_parm.split[2]) {
//       state(i, j, k, URHO) = prob_parm.rho_l;
//       state(i, j, k, UMX) = 0.0;
//       state(i, j, k, UMY) = 0.0;
//       state(i, j, k, UMZ) = prob_parm.rho_l * prob_parm.u_l;
//       state(i, j, k, UEDEN) = prob_parm.rhoe_l + 0.5 * prob_parm.rho_l *
//                                                    prob_parm.u_l *
//                                                    prob_parm.u_l;
//       state(i, j, k, UEINT) = prob_parm.rhoe_l;
//       state(i, j, k, UTEMP) = prob_parm.T_l;
//       state(i, j, k, UFS + prob_parm.left_gas_id) = state(i, j, k, URHO);
//     } else {
//       state(i, j, k, URHO) = prob_parm.rho_r;
//       state(i, j, k, UMX) = 0.0;
//       state(i, j, k, UMY) = 0.0;
//       state(i, j, k, UMZ) = prob_parm.rho_r * prob_parm.u_r;
//       state(i, j, k, UEDEN) = prob_parm.rhoe_r + 0.5 * prob_parm.rho_r *
//                                                    prob_parm.u_r *
//                                                    prob_parm.u_r;
//       state(i, j, k, UEINT) = prob_parm.rhoe_r;
//       state(i, j, k, UTEMP) = prob_parm.T_r;
//       state(i, j, k, UFS + prob_parm.right_gas_id) = state(i, j, k, URHO);
//     }
//   } else {
//     amrex::Abort("invalid idir");
//   }
// }

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

// void pc_prob_close();

inline void pc_prob_close() {}

using ProblemSpecificFunctions = DefaultProblemSpecificFunctions;

#endif
