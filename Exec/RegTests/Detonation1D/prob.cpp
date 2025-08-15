#include <AMReX.H>
#include <PeleC.H>
#include <IndexDefines.H>
#include <EOS.H>
#include <prob_parm.H>   // <-- ensure ProbParm is declared
#include "prob.H"

using namespace amrex;

AMREX_GPU_DEVICE
AMREX_FORCE_INLINE
void
pc_initdata(int i, int j, int k,
            Array4<Real> const& state,
            GeometryData const& geomdata,
            ProbParm const& /*prob*/)
{
    const auto problo = geomdata.ProbLo();
    const auto dx = geomdata.CellSize();
    const Real x = problo[0] + (i + Real(0.5)) * dx[0];

    // Define left and right state
    Real T, P;
    Real Y[NUM_SPECIES] = {0.0};

    if (x < Real(0.05)) {
        // Hot ignition zone
        T = Real(2500.0);
        P = Real(2.0e6);
        Y[0]  = Real(2.0/3.0);  // H2 (adjust index for your mechanism)
        Y[11] = Real(1.0/3.0);  // O2
    } else {
        // Unburned cold gas
        T = Real(300.0);
        P = Real(1.0e5);
        Y[0]  = Real(2.0/3.0);  // H2
        Y[11] = Real(1.0/3.0);  // O2
    }

    auto eos = pele::physics::PhysicsType::eos();

    Real rho, eint;
    // Modern PelePhysics API: (P, Y, T) -> (rho, e)
    eos.PYT2RE(P, Y, T, rho, eint);

    state(i,j,k,URHO ) = rho;
    state(i,j,k,UEINT) = rho * eint;
    state(i,j,k,UTEMP) = T;
    state(i,j,k,UMX  ) = Real(0.0);
    state(i,j,k,UMY  ) = Real(0.0);
    state(i,j,k,UMZ  ) = Real(0.0);

    for (int n = 0; n < NUM_SPECIES; ++n) {
        state(i,j,k,UFS+n) = rho * Y[n];
    }
}

// Optionals can remain empty if your case doesn't need them
void PeleC::problem_post_init() {}
void PeleC::problem_post_timestep() {}
void PeleC::problem_post_restart() {}
