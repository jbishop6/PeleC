#include <AMReX_EB2.H>
#include <AMReX_EB2_IF_Box.H>
#include <AMReX_EB2_GeometryShop.H>
#include <AMReX_ParmParse.H>
#include <AMReX_REAL.H>

using namespace amrex;

EB2::GeometryShop<EB2::BoxIF> makeGeometry()
{
    // Read geometry parameters from input file
    RealArray box_lo, box_hi;
    int has_fluid_inside = 0;

    ParmParse pp("eb2");
    pp.getarr("box_lo", box_lo, 0, AMREX_SPACEDIM);       // e.g., 0.4 0.03
    pp.getarr("box_hi", box_hi, 0, AMREX_SPACEDIM);       // e.g., 0.6 0.09
    pp.query("box_has_fluid_inside", has_fluid_inside);   // 0 or 1

    bool inside = (has_fluid_inside != 0);

    // Build a simple box implicit function
    EB2::BoxIF box(box_lo, box_hi, inside);

    // Wrap it into a GeometryShop and return
    return EB2::makeShop(box);
}
