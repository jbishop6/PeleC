#include <AMReX_EB2.H>
#include <AMReX_EB2_GeometryShop.H>
#include <AMReX_ParmParse.H>
#include <AMReX_Print.H>
#include "TwoBranchIF.H"  // 👈 make sure this file is in your include path

using namespace amrex;

// You can also typedef the return type to make it cleaner
EB2::GeometryShop<TwoBranchIF> makeGeometry()
{
    ParmParse pp("geo");

    Real W, H, L, xs, xr, mid, cL, cR;
    pp.get("W",   W);
    pp.get("H",   H);
    pp.get("L",   L);
    pp.get("xs",  xs);
    pp.get("xr",  xr);
    pp.get("mid", mid);
    pp.get("cL",  cL);
    pp.get("cR",  cR);

    // Get grid spacing to calculate EB features relative to grid
    ParmParse ppa("amr");
    Vector<int> n_cell;
    ppa.getarr("n_cell", n_cell, 0, AMREX_SPACEDIM);

    Geometry const& geom = amrex::DefaultGeometry();
    RealBox const& prob_domain = geom.ProbDomain();
    const Real* prob_lo = prob_domain.lo();
    const Real* prob_hi = prob_domain.hi();

    Real dx = (prob_hi[0] - prob_lo[0]) / static_cast<Real>(n_cell[0]);
    Real dy = (prob_hi[1] - prob_lo[1]) / static_cast<Real>(n_cell[1]);

    Print() << "[EB] TwoBranch connect: "
            << "xs=" << xs << " xr=" << xr << " mid=" << mid
            << " cL=" << cL << " cR=" << cR
            << " dx=" << dx << " dy=" << dy << "\n";

    TwoBranchIF iface(W, H, L, xs, xr, mid, cL, cR, dx, dy);
    return EB2::GeometryShop<TwoBranchIF>(iface);
}

void setupEBGeometry(const Geometry& geom, int required_level, int max_level)
{
    // ✅ Only call this once
    EB2::Initialize();

    auto gshop = makeGeometry();  // 👈 Your custom geometry
    EB2::Build(gshop, geom, required_level, max_level);

    Print() << "[EB] Geometry built successfully.\n";
}
