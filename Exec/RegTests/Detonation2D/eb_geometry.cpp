#include <AMReX_EB2.H>
#include <AMReX_EB2_IF.H>
#include <AMReX_EB2_GeometryShop.H>
#include <AMReX_Print.H>
#include <AMReX_Array.H>

using namespace amrex;

// Example 2D embedded boundary geometry
EB2::BoxIF makeGeometry()
{
    RealArray lo = {0.4, 0.03};
    RealArray hi = {0.6, 0.09};
    return EB2::BoxIF(lo, hi, false);  // false = solid in the box
}

void setupEBGeometry(const Geometry& geom, int required_level, int max_level)
{
    amrex::Print() << "[EB] setupEBGeometry: Initialize + Build\n";

    EB2::Initialize();  // ✅ AMReX 24.06+ requires no arguments

    auto shop = EB2::makeShop(makeGeometry());  // Wrap the implicit function
    EB2::Build(shop, geom, required_level, max_level);
}
