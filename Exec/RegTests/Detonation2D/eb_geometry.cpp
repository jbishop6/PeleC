#include <AMReX.H>
#include <AMReX_EB2.H>
#include <AMReX_EB2_IF_Box.H>
#include <AMReX_EB2_GeometryShop.H>

using namespace amrex;

EB2::GeometryShop<EB2::BoxIF> makeGeometry()
{
    ParmParse pp("eb2");
    RealArray lo, hi;
    pp.getarr("box_lo", lo, 0, AMREX_SPACEDIM);
    pp.getarr("box_hi", hi, 0, AMREX_SPACEDIM);

    const bool has_fluid_inside = false;
    EB2::BoxIF box({lo[0], lo[1]}, {hi[0], hi[1]}, has_fluid_inside);

    amrex::Print() << "[EB] Drawing BoxIF from "
                   << lo[0] << "," << lo[1] << " to " << hi[0] << "," << hi[1] << "\n";

    return EB2::GeometryShop<EB2::BoxIF>(box);
}

void setupEBGeometry(const Geometry& geom, int required_level, int max_level)
{
    EB2::Initialize();  // Only needed once per run

    auto shop = EB2::makeShop(makeGeometry());
    EB2::Build(shop, geom, required_level, max_level);

    amrex::Print() << "[EB] Geometry built successfully.\n";
}
