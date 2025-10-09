#include <AMReX_EB2.H>
#include <AMReX_EB2_IF.H>
#include <AMReX_EB2_IF_Union.H>
#include <AMReX_EB2_GeometryShop.H>
#include <AMReX_ParmParse.H>
#include <AMReX_Print.H>
#include <algorithm>

using namespace amrex;
int eb_geometry_dummy_symbol = 42;

void makeGeometry(const Geometry& geom, int required_coarsening_level, int max_coarsening_level)
{
    // Define a rotated box embedded boundary in 2D
    EB2::BoxIF box({0.2, 0.2, 0.0}, {0.8, 0.8, 0.0}, true);  // Z=0 for 2D

    GpuArray<Real, AMREX_SPACEDIM> theta = {0.0, 0.0, M_PI / 4.0};  // 45° Z-rotation

    auto rotated = EB2::translate(
                       EB2::rotate(box, theta),
                       {0.0, 0.0, 0.0});  // No shift

    auto gshop = EB2::makeShop(rotated);
    EB2::Build(gshop, geom, required_coarsening_level, max_coarsening_level);
}

void setupEBGeometry(const amrex::Geometry& geom, int required, int max)
{
    makeGeometry(geom, required, max);
}
