#include <AMReX_EB2.H>
#include <AMReX_EB2_IF_Box.H>
#include <AMReX_EB2_GeometryShop.H>
#include <AMReX_Print.H>
#include <AMReX_Array.H>

using namespace amrex;

void makeGeometry(const Geometry& geom, int required_coarsening_level, int max_coarsening_level)
{
    amrex::Print() << "[EB] makeGeometry() — minimal safe box\n";

    // Domain: [0.0, 1.0] x [0.0, 0.125] — stay well inside
    RealArray lo = {0.3, 0.04};
    RealArray hi = {0.4, 0.05};  // only 1cm tall, in middle of Y

    EB2::BoxIF box(lo, hi, false);  // false = solid region

    auto gshop = EB2::makeShop(box);
    EB2::Build(gshop, geom, required_coarsening_level, max_coarsening_level);
}

void setupEBGeometry(const Geometry& geom, int required, int max)
{
    makeGeometry(geom, required, max);
}
