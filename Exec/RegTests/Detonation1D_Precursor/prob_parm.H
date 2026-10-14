
#ifndef PROB_PARM_H
#define PROB_PARM_H

#include <AMReX_REAL.H>
#include <AMReX_GpuQualifiers.H>
#include <AMReX_GpuMemory.H>
#include "mechanism.H"

struct ProbParmDevice
{
    // Right state: fresh ethylene-air
    amrex::Real p_r = -1.0;
    amrex::Real u_r = 0.0;
    amrex::Real rho_r = 0.0;
    amrex::Real rhoe_r = 0.0;
    amrex::Real T_r = -1.0;
    amrex::Real Y_r[NUM_SPECIES] = {0.0};

    // Left state: initiation region
    amrex::Real p_l = -1.0;
    amrex::Real u_l = 0.0;
    amrex::Real rho_l = 0.0;
    amrex::Real rhoe_l = 0.0;
    amrex::Real T_l = -1.0;
    amrex::Real Y_l[NUM_SPECIES] = {0.0};

    // Initial interface location in cm
    amrex::Real x_interface = 0.5;
};

struct ProbParmHost
{
    ProbParmHost() = default;
};

#endif
