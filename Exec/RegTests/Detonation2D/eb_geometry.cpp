#include <AMReX_EB2.H>
#include <AMReX_EB2_IF_Box.H>
#include <AMReX_EB2_GeometryShop.H>
#include <AMReX_ParmParse.H>
#include <AMReX_REAL.H>

using namespace amrex;

EB2::GeometryShop<EB2::BoxIF> makeGeometry()
{
    // Use std::vector to read input parameters
    std::vector<Real> lo_vec, hi_vec;
    int has_fluid_inside = 0;

    ParmParse pp("eb2");
    pp.getarr("box_lo", lo_vec);
    pp.getarr("box_hi", hi_vec);
    pp.query("box_has_fluid_inside", has_fluid_inside);

    bool inside = (has_fluid_inside != 0);

    // Convert vectors -> RealArray (fixed-size)
    RealArray box_lo = {AMREX_D_DECL(lo_vec[0], lo_vec[1], 0.0)};
    RealArray box_hi = {AMREX_D_DECL(hi_vec[0], hi_vec[1], 0.0)};

    // Build a simple box implicit function
    EB2::BoxIF box(box_lo, box_hi, inside);

    // Wrap into a GeometryShop and return
    return EB2::makeShop(box);
}
