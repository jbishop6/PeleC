#include <AMReX_EB2.H>
#include <AMReX_EB2_IF_Box.H>
#include <AMReX_EB2_IF_Rotation.H>
#include <AMReX_EB2_GeometryShop.H>
#include <AMReX_Print.H>
#include <AMReX_Array.H>

using namespace amrex;

void makeGeometry(const Geometry& geom, int required_coarsening_level, int max_coarsening_level)
{
    amrex::Print() << "[EB] makeGeometry() — safe 2D box in domain\n";

    // Safe bounds in your domain:
    // X ∈ [0.4, 0.6] — matches your eb2.box_lo/hi
    // Y ∈ [0.03, 0.09] — inside [0, 0.125]
    RealArray lo = {0.40, 0.03};
    RealArray hi = {0.60, 0.09};

    // Box is solid (fluid outside)
    EB2::BoxIF box(lo, hi, false);

    // Optional rotation — set to 0.0 for now to avoid trouble
    Real angle = 0.0;  // radians
    int dir = 2;       // z-axis rotation (only axis in 2D)

    auto rotated = EB2::rotate(box, angle, dir);

    auto gshop = EB2::makeShop(rotated);
    EB2::Build(gshop, geom, required_coarsening_level, max_coarsening_level);
}

void setupEBGeometry(const Geometry& geom, int required, int max)
{
    makeGeometry(geom, required, max);
}
