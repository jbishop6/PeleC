#include <AMReX_EB2.H>
#include <AMReX_EB2_IF_Box.H>
#include <AMReX_EB2_IF_Rotation.H>
#include <AMReX_EB2_GeometryShop.H>
#include <AMReX_Print.H>
#include <AMReX_Array.H>

using namespace amrex;

void makeGeometry(const Geometry& geom, int required_coarsening_level, int max_coarsening_level)
{
    amrex::Print() << "[EB] makeGeometry() — building 2D rotated box (no translate).\n";

    // Define lower and upper corners of the box
    RealArray lo = {0.25, 0.25};
    RealArray hi = {0.75, 0.75};

    // Create the EB box shape (false = solid inside)
    EB2::BoxIF box(lo, hi, false);

    // Optional: rotate 30 degrees about z-axis (2D, so dir = 2)
    Real angle = M_PI / 6.0;
    int dir = 2;
    auto rotated = EB2::rotate(box, angle, dir);

    // Build the EB geometry
    auto gshop = EB2::makeShop(rotated);
    EB2::Build(gshop, geom, required_coarsening_level, max_coarsening_level);
}

void setupEBGeometry(const Geometry& geom, int required, int max)
{
    makeGeometry(geom, required, max);
}
