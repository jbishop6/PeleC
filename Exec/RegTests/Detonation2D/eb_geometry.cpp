// Exec/RegTests/Detonation2D/eb_geometry.cpp
#include <AMReX_EB2.H>
#include <AMReX_EB2_IF.H>
#include <AMReX_EB2_GeometryShop.H>
#include <AMReX_ParmParse.H>

using namespace amrex;

// helper to build a BoxIF from 2D bounds (z=0 for 2D build)
static EB2::BoxIF make_box(Real x0, Real y0, Real x1, Real y1, bool has_fluid_inside)
{
    const Real xlo = std::min(x0, x1);
    const Real ylo = std::min(y0, y1);
    const Real xhi = std::max(x0, x1);
    const Real yhi = std::max(y0, y1);

    amrex::Array<Real, AMREX_SPACEDIM> lo{AMREX_D_DECL(xlo, ylo, 0.0)};
    amrex::Array<Real, AMREX_SPACEDIM> hi{AMREX_D_DECL(xhi, yhi, 0.0)};

    // Older AMReX: BoxIF(lo, hi, has_fluid_inside)
    return EB2::BoxIF(lo, hi, has_fluid_inside);
}

void Initialize_EB2 (const Geometry& geom,
                     int required_coarsening_level,
                     int max_coarsening_level)
{
    using namespace EB2;

    // ---- User parameters (domain units) ----
    ParmParse pp("geo");
    Real W   = 0.05;     // base duct height
    Real H   = 0.05;     // upward offset for upper branch
    Real L   = 0.05;     // downward offset for lower branch
    Real xs  = 0.25;     // x where branches start (split)
    Real xr  = 0.85;     // x where branches rejoin
    Real t   = 0.004;    // vertical connector thickness
    pp.query("W",  W);
    pp.query("H",  H);
    pp.query("L",  L);
    pp.query("xs", xs);
    pp.query("xr", xr);
    pp.query("t",  t);

    const RealBox& rb = geom.ProbDomain();
    const Real xlo = rb.lo(0), xhi = rb.hi(0);
    const Real ylo = rb.lo(1), yhi = rb.hi(1);
    const Real ymid = 0.5*(ylo+yhi);

    // Clamp to domain / avoid degenerate connectors
    xs = std::max(xlo, std::min(xs, xhi));
    xr = std::max(xs + 1e-12, std::min(xr, xhi));
    t  = std::max(1e-6*(xhi-xlo), std::min(t, xr-xs - 1e-12));

  // --- Fluid rectangles (has_fluid_inside = true) ---

// Main duct only to the LEFT of xs and to the RIGHT of xr
auto left_main  = make_box(xlo, ymid-0.5*W, xs, ymid+0.5*W, true);
auto right_main = make_box(xr,  ymid-0.5*W, xhi, ymid+0.5*W, true);

// Upper branch run (raised by +H) between xs and xr
auto upper_run = make_box(xs,  ymid-0.5*W+H, xr,  ymid+0.5*W+H, true);

// Lower branch run (lowered by -L) between xs and xr
auto lower_run = make_box(xs,  ymid-0.5*W-L, xr,  ymid+0.5*W-L, true);

// Vertical connectors at split (xs) and rejoin (xr)
auto up_conn_L = make_box(xs,     ymid+0.5*W,     xs+t, ymid+0.5*W+H, true);
auto up_conn_R = make_box(xr-t,   ymid+0.5*W,     xr,   ymid+0.5*W+H, true);
auto lo_conn_L = make_box(xs,     ymid-0.5*W-L,   xs+t, ymid-0.5*W,   true);
auto lo_conn_R = make_box(xr-t,   ymid-0.5*W-L,   xr,   ymid-0.5*W,   true);

// Pairwise unions (older AMReX prefers chaining)
auto u1 = EB2::makeUnion(left_main, right_main);
auto u2 = EB2::makeUnion(u1, upper_run);
auto u3 = EB2::makeUnion(u2, lower_run);
auto u4 = EB2::makeUnion(u3, up_conn_L);
auto u5 = EB2::makeUnion(u4, up_conn_R);
auto u6 = EB2::makeUnion(u5, lo_conn_L);
auto channel = EB2::makeUnion(u6, lo_conn_R);

// Walls = complement of the fluid channel
auto walls = EB2::makeComplement(channel);


    // pick something reasonable
    int max_grid_size = 128;   // 64–256 is fine
    int ngrow         = 4;

    // use the overload that takes the Geometry
    EB2::Build(gshop, geom,
           required_coarsening_level,
           max_coarsening_level,
           max_grid_size,
           ngrow);

}
