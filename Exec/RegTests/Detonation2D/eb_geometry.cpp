#include <AMReX_EB2.H>
#include <AMReX_EB2_IF_Box.H>
#include <AMReX_EB2_GeometryShop.H>
#include <AMReX_ParmParse.H>
#include <AMReX_REAL.H>

using namespace amrex;

EB2::GeometryShop<EB2::BoxIF> makeGeometry()
{
    ParmParse pp("eb2");

    RealArray lo, hi;
    pp.getarr("box_lo", lo, 0, AMREX_SPACEDIM);
    pp.getarr("box_hi", hi, 0, AMREX_SPACEDIM);
    int has_fluid_inside = 1;
    pp.query("box_has_fluid_inside", has_fluid_inside);

    EB2::BoxIF box(lo, hi, static_cast<bool>(has_fluid_inside));
    return EB2::makeShop(box);
}


void setupEBGeometry(const Geometry& geom, int required_level, int max_level)
{
    EB2::Initialize();

    auto shop = makeGeometry();  // Now valid!
    EB2::Build(shop, geom, required_level, max_level);

    amrex::Print() << "[EB] setupEBGeometry: Initialize + Build\n";
}
