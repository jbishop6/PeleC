// Exec/RegTests/Detonation1D/prob.cpp
#include "prob.H"
#include <AMReX_ParmParse.H>

using namespace amrex;

// REQUIRED by PeleC: C-linkage hook called during setup
extern "C"
void amrex_probinit (const int* /*init*/,
                     const int* /*name*/,
                     const amrex_real* /*problo*/,
                     const amrex_real* /*probhi*/)
{
    // Parse problem params here if you want (safe to leave empty)
    ParmParse pp("prob");
    // e.g., int idir; pp.query("idir", idir); ...
}

// Optional hooks (safe no-ops)
void PeleC::problem_post_init() {}
void PeleC::problem_post_timestep() {}
void PeleC::problem_post_restart() {}
