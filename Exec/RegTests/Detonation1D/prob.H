#ifndef _PROB_H_
#define _PROB_H_

#include <AMReX_REAL.H>
#include <AMReX_GpuQualifiers.H>
#include <IndexDefines.H>
#include <EOS.H>

struct ProbParm {};
struct ProbParmDevice {};

namespace ProblemSpecificFunctions {

AMREX_GPU_DEVICE AMREX_FORCE_INLINE
void problem_initialize(int i, int j, int k,
                        amrex::Array4<amrex::Real> const& state,
                        const amrex::GeometryData& geomdata,
                        const ProbParmDevice& prob)
{
    const auto problo = geomdata.ProbLo();
    const auto dx = geomdata.CellSize();
    const amrex::Real x = problo[0] + (i + 0.5) * dx[0];

    amrex::Real T, P;
    amrex::Real Y[NUM_SPECIES] = {0.0};

    if (x < 0.1) {
        T = 2500.0;
        P = 2.0e6;
        Y[0] = 2.0 / 3.0;
        Y[1] = 1.0 / 3.0;
    } else {
        T = 300.0;
        P = 1.0e5;
        Y[1] = 0.21;
        Y[2] = 0.79;
    }

    auto eos = pele::physics::PhysicsType::eos();
    amrex::Real rho, eint;
    eos.RPY2RhoE(P, T, Y, rho, eint);

    state(i,j,k,URHO ) = rho;
    state(i,j,k,UEINT) = rho * eint;
    state(i,j,k,UTEMP) = T;
    state(i,j,k,UMX  ) = 0.0;
    state(i,j,k,UMY  ) = 0.0;
    state(i,j,k,UMZ  ) = 0.0;

    for (int n = 0; n < NUM_SPECIES; ++n) {
        state(i,j,k,UFS+n) = rho * Y[n];
    }
}

AMREX_GPU_DEVICE AMREX_FORCE_INLINE
void problem_modify_transport_coeffs(...) {
    // Optional: You can leave this empty for now
}

AMREX_GPU_DEVICE AMREX_FORCE_INLINE
void problem_eb_state(...) {
    // Optional: You can leave this empty if not using EB
}

AMREX_GPU_DEVICE AMREX_FORCE_INLINE
void set_problem_tags(...) {
    // Optional tagging logic
}

} // namespace ProblemSpecificFunctions

#endif
