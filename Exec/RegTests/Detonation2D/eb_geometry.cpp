#include <AMReX_EB2.H>
#include <AMReX_EB2_IF_Box.H>
#include <AMReX_ParmParse.H>

using namespace amrex;

EB2::GeometryShop<EB2::BoxIF> makeGeometry()
{
    ParmParse pp("eb2");

    // Use std::vector to hold raw input first
    std::vector<Real> lo_vec, hi_vec;
    pp.getarr("box_lo", lo_vec, 0, AMREX_SPACEDIM);
    pp.getarr("box_hi", hi_vec, 0, AMREX_SPACEDIM);

    // Copy to RealArray
    RealArray lo, hi;
    for (int i = 0; i < AMREX_SPACEDIM; ++i) {
        lo[i] = lo_vec[i];
        hi[i] = hi_vec[i];
    }

    EB2::BoxIF box(lo, hi, false);  // false = fluid outside
    return EB2::makeShop(box);
}

void setupEBGeometry(const amrex::Geometry& geom, int required_level, int max_level)
{
    // (1) Initialize EB2
    amrex::EB2::Initialize();

    // (2) Build the geometry shop (your makeGeometry() function)
    auto shop = makeGeometry();  // No arguments

    // (3) Build EB2
    amrex::EB2::Build(shop, geom, required_level, max_level);
}

