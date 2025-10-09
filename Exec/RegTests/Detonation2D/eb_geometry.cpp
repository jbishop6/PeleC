#include <AMReX_EB2.H>
#include <AMReX_EB2_GeometryShop.H>
#include <AMReX_EB2_IF_Box.H>
#include <AMReX_ParmParse.H>

using namespace amrex;

// Your custom TwoBranch geometry
EB2::GeometryShop<EB2::IF<Box2D>> makeGeometry()
{
    Real xs, xr, mid, cL, cR, W, H, L;
    ParmParse pp("geo");
    pp.get("xs", xs);
    pp.get("xr", xr);
    pp.get("mid", mid);
    pp.get("cL", cL);
    pp.get("cR", cR);
    pp.get("W", W);
    pp.get("H", H);
    pp.get("L", L);

    // Example: just create a simple box for now to avoid segfaults
    RealArray lo = {xs, 0.0};
    RealArray hi = {xr, H};
    auto box = EB2::BoxIF(lo, hi, false);  // false: fluid outside

    return EB2::makeShop(box);
}
