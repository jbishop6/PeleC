#include <AMReX_EB2.H>
#include <AMReX_EB2_IF_Box.H>
#include <AMReX_EB2_IF_Rotation.H>
#include <AMReX_EB2_IF_Transform.H>   // ✅ For translate()
#include <AMReX_EB2_GeometryShop.H>
#include <AMReX_Print.H>
#include <AMReX_Array.H>              // ✅ For RealArray

using namespace amrex;

void makeGeometry(const Geometry& geom, int required_coarsening_level, int max_coarsening_level)
{
    amrex::Print() << "[EB] makeGeometry() — building 2D rotated box.\n";

    // Define lower and upper corners of the box
    RealArray lo = {0.25, 0.25};
    RealArray hi = {0.75, 0.75};

    // Make the EB shape: a solid box (fluid is outside)
    EB2::BoxIF box(lo, hi, false);

    // Rotate the box around the Z-axis (dir=2)
    Real angle = M_PI / 6.0;  // 30 degrees
    int dir = 2;

    auto rotated = EB2::rotate(box, angle, dir);

    // Optional translation (none here)
    RealArray shift = {0.0, 0.0};
    auto translated = EB2::translate(rotated, shift);

    // Finalize EB shape
    auto gshop = EB2::makeShop(translated);
    EB2::Build(gshop, geom, required_coarsening_level, max_coarsening_level);
}

void setupEBGeometry(const Geometry& geom, int required, int max)
{
    makeGeometry(geom, required, max);
}
