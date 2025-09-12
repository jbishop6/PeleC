// Exec/RegTests/Detonation2D/eb_geometry.cpp
#include <AMReX_EB2.H>
#include <AMReX_EB2_IF.H>
#include <AMReX_EB2_GeometryShop.H>
#include <AMReX_ParmParse.H>
#include <AMReX_Print.H>
#include <algorithm>

using namespace amrex;

static EB2::BoxIF make_box(Real x0, Real y0, Real x1, Real y1, bool has_fluid_inside)
{
    const Real xlo = std::min(x0, x1);
    const Real ylo = std::min(y0, y1);
    const Real xhi = std::max(x0, x1);
    const Real yhi = std::max(y0, y1);
    amrex::Array<Real, AMREX_SPACEDIM> lo{AMREX_D_DECL(xlo, ylo, 0.0)};
    amrex::Array<Real, AMREX_SPACEDIM> hi{AMREX_D_DECL(xhi, yhi, 0.0)};
    return EB2::BoxIF(lo, hi, has_fluid_inside);
}

void Initialize_EB2 (const Geometry& geom,
                     int required_coarsening_level,
                     int max_coarsening_level)
{
    using namespace EB2;

    // ---- which geometry (match previous student's style) ----
    ParmParse peb("eb2");
    std::string geom_type = "all_regular";
    peb.query("geom_type", geom_type);

    amrex::Print() << "[EB] geom_type=" << geom_type << "\n";

    // ---------- all regular (no EB) ----------
    if (geom_type == "all_regular" || geom_type == "regular" || geom_type == "none") {
        EB2::AllRegularIF all_reg;
        EB2::GeometryShop gshop(all_reg);
        EB2::Build(gshop, geom, required_coarsening_level, max_coarsening_level, 128, 4);
        return;
    }

    // ---------- two-branch duct (alias: test) ----------
    if (geom_type == "twobranch" || geom_type == "test") {
        ParmParse pp("geo"); // knobs in inputs under "geo."
        Real W=0.05, H=0.03, L=0.03, xs=0.25, xr=0.80, t=0.004;
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

        // clamp / guard
        xs = std::max(xlo, std::min(xs, xhi));
        xr = std::max(xs + 1e-12, std::min(xr, xhi));
        t  = std::max(1e-6*(xhi-xlo), std::min(t, xr - xs - 1e-12));

        amrex::Print() << "[EB] two-branch params: W="<<W<<" H="<<H<<" L="<<L
                       <<" xs="<<xs<<" xr="<<xr<<" t="<<t<<"\n";

        // FLUID channel pieces (has_fluid_inside = true)
        auto left_main  = make_box(xlo, ymid-0.5*W, xs, ymid+0.5*W, true);
        auto right_main = make_box(xr,  ymid-0.5*W, xhi, ymid+0.5*W, true);
        auto upper_run  = make_box(xs,  ymid-0.5*W+H, xr,  ymid+0.5*W+H, true);
        auto lower_run  = make_box(xs,  ymid-0.5*W-L, xr,  ymid+0.5*W-L, true);
        auto up_conn_L  = make_box(xs,     ymid+0.5*W,     xs+t, ymid+0.5*W+H, true);
        auto up_conn_R  = make_box(xr-t,   ymid+0.5*W,     xr,   ymid+0.5*W+H, true);
        auto lo_conn_L  = make_box(xs,     ymid-0.5*W-L,   xs+t, ymid-0.5*W,   true);
        auto lo_conn_R  = make_box(xr-t,   ymid-0.5*W-L,   xr,   ymid-0.5*W,   true);

        // Chain unions (older AMReX prefers pairwise unions)
        auto u1 = EB2::makeUnion(left_main, right_main);
        auto u2 = EB2::makeUnion(u1, upper_run);
        auto u3 = EB2::makeUnion(u2, lower_run);
        auto u4 = EB2::makeUnion(u3, up_conn_L);
        auto u5 = EB2::makeUnion(u4, up_conn_R);
        auto u6 = EB2::makeUnion(u5, lo_conn_L);
        auto channel = EB2::makeUnion(u6, lo_conn_R);

        // SOLIDS = complement of the fluid channel
        auto walls = EB2::makeComplement(channel);

        EB2::GeometryShop gshop(walls);
        EB2::Build(gshop, geom, required_coarsening_level, max_coarsening_level, 128, 4);
        return;
    }

    // ---------- unknown type: fall back with a clear abort ----------
    amrex::Abort(std::string("[EB] Unknown eb2.geom_type: ") + geom_type);
}
