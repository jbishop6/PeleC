#include <AMReX_EB2.H>
#include <AMReX_EB2_IF_Box.H>
#include <AMReX_EB2_IF_Rotation.H>
#include <AMReX_EB2_GeometryShop.H>
#include <AMReX_Print.H>

using namespace amrex;

void makeGeometry(const Geometry& geom, int required_coarsening_level, int max_coarsening_level)
{
    amrex::Print() << "[EB] makeGeometry() — building 2D rotated box.\n";

    // Define lower/upper corners of the box in 2D
    EB2::RealArray lo = {0.25, 0.25};
    EB2::RealArray hi = {0.75, 0.75};

    // Define a solid box (fluid outside)
    EB2::BoxIF box(lo, hi, false);

    // Define a rotation: angle in radians, rotation axis direction
    // For 2D, only one angle is needed (about z-axis, so dir=2)
    Real angle = M_PI / 6.0; // 30 degrees rotation
    int dir = 2;             // rotation axis (z in 3D, but ok in 2D)

    auto rotated_box = EB2::rotate(box, angle, dir);

    // Optionally, translate geometry
    EB2::RealArray shift = {0.0, 0.0};
    auto shifted_box = EB2::translate(rotated_box, shift);

    // Build the geometry
    auto gshop = EB2::makeShop(shifted_box);
    EB2::Build(gshop, geom, required_coarsening_level, max_coarsening_level);
}

void setupEBGeometry(const Geometry& geom, int required, int max)
{
    makeGeometry(geom, required, max);
}
