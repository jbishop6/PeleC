#include <AMReX_EB2.H>
#include <AMReX_EB2_IF.H>
#include <AMReX_EB2_GeometryShop.H>
#include <AMReX_Print.H>
#include <AMReX_Array.H>

using namespace amrex;

EB2::BoxIF makeGeometry()
{
    RealArray lo = {0.4, 0.03};
    RealArray hi = {0.6, 0.09};
    return EB2::BoxIF(lo, hi, false);
}

void setupEBGeometry(const Geometry& geom, int required_level, int max_level)
{
    amrex::Print() << "[EB] setupEBGeometry: calling EB2::Initialize + Build\n";

    EB2::Initialize(geom, required_level, max_level);            // ✅ Correct call
    auto shop = EB2::makeShop(makeGeometry());                   // ✅ Wrap implicit function
    EB2::Build(shop, geom, required_level, max_level);           // ✅ Register geometry
}
