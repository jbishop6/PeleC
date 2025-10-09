#include <AMReX_EB2.H>
#include <AMReX_EB2_IF_Box.H>
#include <AMReX_EB2_GeometryShop.H>
#include <AMReX_ParmParse.H>

using namespace amrex;

EB2::GeometryShop<EB2::BoxIF> makeGeometry()
{
    RealArray lo = {0.4, 0.03};  // lower corner of box
    RealArray hi = {0.6, 0.09};  // upper corner of box
    bool has_fluid_inside = false;

    // Print for debug
    amrex::Print() << "[EB] Drawing BoxIF from " << lo[0] << "," << lo[1]
                   << " to " << hi[0] << "," << hi[1] << "\n";

    EB2::BoxIF box(lo, hi, has_fluid_inside);
    EB2::GeometryShop<EB2::BoxIF> shop(box);
    return shop;
}

void setupEBGeometry(const Geometry& geom, int required_level, int max_level)
{
    EB2::Initialize();  // only once per run

    auto shop = makeGeometry();  // calls the BoxIF function above

    EB2::Build(shop, geom, required_level, max_level);

    amrex::Print() << "[EB] Geometry built successfully.\n";
}
