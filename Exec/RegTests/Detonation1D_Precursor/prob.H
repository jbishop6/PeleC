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
    // Cell-center position in cm
    const amrex::Real x =
        geomdata.ProbLo(0) +
        (i + 0.5) * geomdata.CellSize(0);

    const bool left =
        (x < prob_parm.x_interface);

    const amrex::Real rho =
        left ? prob_parm.rho_l : prob_parm.rho_r;

    const amrex::Real u =
        left ? prob_parm.u_l : prob_parm.u_r;

    const amrex::Real rhoe =
        left ? prob_parm.rhoe_l : prob_parm.rhoe_r;

    const amrex::Real T =
        left ? prob_parm.T_l : prob_parm.T_r;

    state(i, j, k, URHO) = rho;
    state(i, j, k, UMX) = rho * u;

#if AMREX_SPACEDIM >= 2
    state(i, j, k, UMY) = 0.0;
#endif

#if AMREX_SPACEDIM == 3
    state(i, j, k, UMZ) = 0.0;
#endif

    state(i, j, k, UEINT) = rhoe;

    state(i, j, k, UEDEN) =
        rhoe + 0.5 * rho * u * u;

    state(i, j, k, UTEMP) = T;

    for (int n = 0; n < NUM_SPECIES; ++n) {
        const amrex::Real Y =
            left ? prob_parm.Y_l[n]
                 : prob_parm.Y_r[n];

        state(i, j, k, UFS + n) = rho * Y;
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

