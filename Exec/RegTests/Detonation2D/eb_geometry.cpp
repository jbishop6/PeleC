// Exec/RegTests/Detonation2D/eb_geometry.cpp
#include <AMReX_EB2.H>
#include <AMReX_EB2_IF.H>
#include <AMReX_EB2_IF_Union.H>
#include <AMReX_EB2_GeometryShop.H>
#include <AMReX_ParmParse.H>
#include <AMReX_Print.H>
#include <algorithm>

using namespace amrex;

// small helper: FLUID rectangle in 2D
static EB2::BoxIF fluid_box(Real x0, Real y0, Real x1, Real y1)
{
    Array<Real,AMREX_SPACEDIM> lo{AMREX_D_DECL(std::min(x0,x1),
                                               std::min(y0,y1), 0.0)};
    Array<Real,AMREX_SPACEDIM> hi{AMREX_D_DECL(std::max(x0,x1),
                                               std::max(y0,y1), 0.0)};
    return EB2::BoxIF(lo, hi, /*has_fluid_inside=*/true);
}

void Initialize_EB2 (const Geometry& geom,
                     int required_coarsening_level,
                     int max_coarsening_level)
{
    // --- user knobs (domain units) ---
    ParmParse pp("geo");
    Real W=0.04, H=0.05, L=0.05, xs=0.30, xr=0.70, t=0.01; // width, branch offsets, split/rejoin, slit thickness
    pp.query("W",W); pp.query("H",H); pp.query("L",L);
    pp.query("xs",xs); pp.query("xr",xr); pp.query("t",t);

    const RealBox& rb = geom.ProbDomain();
    const Real xlo = rb.lo(0), xhi = rb.hi(0);
    const Real ylo = rb.lo(1), yhi = rb.hi(1);
    const Real ymid = 0.5*(ylo+yhi);

    // guard for geometry resolution & avoiding multi-cuts
    const Real dx = geom.CellSize(0);
    const Real min_gap = 4*dx;
    xs = std::max(xlo, std::min(xs, xhi));
    xr = std::max(xs + min_gap, std::min(xr, xhi));
    t  = std::max(min_gap, std::min(t, xr - xs - min_gap));

    // base duct band
    const Real y_base_lo   = ymid - 0.5*W;
    const Real y_base_hi   = ymid + 0.5*W;
    // branch bands
    const Real y_upper_lo  = y_base_hi;
    const Real y_upper_hi  = y_base_hi + H;
    const Real y_lower_lo  = y_base_lo - L;
    const Real y_lower_hi  = y_base_lo;

    // Fluid pieces
    auto left_main   = fluid_box(xlo, y_base_lo, xs,  y_base_hi);
    auto right_main  = fluid_box(xr,  y_base_lo, xhi, y_base_hi);
    auto upper_run   = fluid_box(xs,  y_upper_lo, xr, y_upper_hi);
    auto lower_run   = fluid_box(xs,  y_lower_lo, xr, y_lower_hi);

    // fluid “slits” connecting base to branches at xs and xr
    auto up_conn_L   = fluid_box(xs,     y_base_hi,  xs + t,  y_upper_hi);
    auto up_conn_R   = fluid_box(xr - t, y_base_hi,  xr,      y_upper_hi);
    auto low_conn_L  = fluid_box(xs,     y_lower_lo, xs + t,  y_base_lo);
    auto low_conn_R  = fluid_box(xr - t, y_lower_lo, xr,      y_base_lo);

    // Union all FLUID parts (pairwise to be AMReX-EB2 friendly)
    auto u1 = amrex::EB2::makeUnion(left_main, right_main);
    auto u2 = amrex::EB2::makeUnion(u1, upper_run);
    auto u3 = amrex::EB2::makeUnion(u2, lower_run);
    auto u4 = amrex::EB2::makeUnion(u3, up_conn_L);
    auto u5 = amrex::EB2::makeUnion(u4, up_conn_R);
    auto u6 = amrex::EB2::makeUnion(u5, low_conn_L);
    auto fluid_channel = amrex::EB2::makeUnion(u6, low_conn_R);

    // SOLIDS = complement of fluid
    auto walls = amrex::EB2::makeComplement(fluid_channel);

    amrex::Print() << "[EB] Two-branch (BoxIF union as FLUID, then complement)  "
                   << "W="<<W<<" H="<<H<<" L="<<L
                   << " xs="<<xs<<" xr="<<xr<<" t="<<t<<" dx="<<dx << "\n";

    auto gshop = amrex::EB2::makeShop(walls);
    // ngrow=4 is typical; extend_domain_face=false to keep walls at domain BC
    amrex::EB2::Build(gshop, geom,
                      required_coarsening_level, max_coarsening_level,
                      /*ngrow*/4, /*extend_domain_face*/false);
}
