#include <AMReX_EB2.H>
#include <AMReX_EB2_IF.H>
#include <AMReX_EB2_GeometryShop.H>
#include <AMReX_Print.H>

using namespace amrex;

// Custom 2D box geometry (solid wall in center)
EB2::BoxIF makeGeometry()
{
    RealArray lo = {0.4, 0.03};
    RealArray hi = {0.6, 0.09};
    return EB2::BoxIF(lo, hi, false);  // fluid outside box
}

void setupEBGeometry(const Geometry& geom, int required_level, int max_level)
{
    amrex::Print() << "[EB] setupEBGeometry: calling Build\n";

    // Required
    Initialize_EB2(geom, required_level, max_level);

    // Build your geometry and register it
    auto shop = EB2::makeShop(makeGeometry());
    EB2::Build(shop, geom, required_level, max_level);
}
