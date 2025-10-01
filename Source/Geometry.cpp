#include "Geometry.H"

#include <AMReX_EB2.H>
#include <AMReX_EB2_IF.H>
#include <AMReX_EB2_IF_Union.H>
#include <AMReX_EB2_GeometryShop.H>
#include <AMReX_ParmParse.H>
#include <AMReX_Print.H>


namespace pele::pelec {

void
FlatPlate::build(
  const amrex::Geometry& /*geom*/, const int /*max_coarsening_level*/)
{
  amrex::Print() << "flat plate  geometry not currently supported. \n";
  amrex::Abort();
}

void
Ramp::build(const amrex::Geometry& geom, const int max_coarsening_level)
{
  amrex::Print() << "ramp geometry\n";
  int upDir;
  int indepVar;
  amrex::Real startPt;
  amrex::Real slope;
  amrex::ParmParse pp("eb2");
  pp.get("up_dir", upDir);
  pp.get("indep_var", indepVar);
  pp.get("start_pt", startPt);
  pp.get("ramp_slope", slope);

  amrex::RealArray normal;
  normal.fill(0.0);
  normal[upDir] = 1.0;
  normal[indepVar] = -slope;

  amrex::RealArray point;
  point.fill(0.0);
  point[upDir] = -slope * startPt;

  amrex::EB2::PlaneIF ramp(point, normal);
  auto gshop = amrex::EB2::makeShop(ramp);
  amrex::EB2::Build(gshop, geom, max_coarsening_level, max_coarsening_level);
}

void
Combustor::build(const amrex::Geometry& geom, const int max_coarsening_level)
{
  amrex::ParmParse pp("combustor");

  amrex::Real fwl;
  pp.get("far_wall_loc", fwl);

  amrex::EB2::PlaneIF farwall(
    {AMREX_D_DECL(fwl, 0., 0.)}, {AMREX_D_DECL(1., 0., 0.)});

  amrex::Vector<amrex::Real> pl1pt;
  amrex::Vector<amrex::Real> pl2pt;
  amrex::Vector<amrex::Real> pl2nm;
  amrex::Vector<amrex::Real> pl3pt;
  pp.getarr("ramp_plane1_point", pl1pt);
  pp.getarr("ramp_plane2_point", pl2pt);
  pp.getarr("ramp_plane2_normal", pl2nm);
  pp.getarr("ramp_plane3_point", pl3pt);

  amrex::EB2::PlaneIF r0(
    {AMREX_D_DECL(pl1pt[0], pl1pt[1], 0.)}, {AMREX_D_DECL(0., -1., 0.)});
  amrex::EB2::PlaneIF r1(
    {AMREX_D_DECL(pl2pt[0], pl2pt[1], 0.)},
    {AMREX_D_DECL(pl2nm[0], pl2nm[1], 0.)});
  amrex::EB2::PlaneIF r2(
    {AMREX_D_DECL(pl3pt[0], pl3pt[1], 0.)}, {AMREX_D_DECL(1., 0., 0.)});
  auto ramp = amrex::EB2::makeIntersection(r0, r1, r2);

  amrex::Vector<amrex::Real> pipelo;
  amrex::Vector<amrex::Real> pipehi;
  pp.getarr("pipe_lo", pipelo);
  pp.getarr("pipe_hi", pipehi);

  amrex::EB2::BoxIF pipe(
    {AMREX_D_DECL(pipelo[0], pipelo[1], -1.)},
    {AMREX_D_DECL(pipehi[0], pipehi[1], 1.)}, false);

  // where does plane 1 and plane 2 intersect?
  amrex::Real k2 = std::abs(pl2nm[0] / pl2nm[1]);
  amrex::Real secty = pl2pt[1] + k2 * (pl3pt[0] - pl2pt[0]);
  // How much do we cut?
  amrex::Real dx = geom.CellSize(0);
  amrex::Real dycut =
    4. * (1. + max_coarsening_level) * amrex::min<amrex::Real>(dx, k2 * dx);
  amrex::EB2::BoxIF flat_corner(
    {AMREX_D_DECL(pl3pt[0], 0., -1.)}, {AMREX_D_DECL(1.e10, secty + dycut, 1.)},
    false);

  auto polys = amrex::EB2::makeUnion(farwall, ramp, pipe, flat_corner);

  amrex::Real lenx = amrex::DefaultGeometry().ProbLength(0);
  amrex::Real leny = amrex::DefaultGeometry().ProbLength(1);
  auto pr = amrex::EB2::translate(
    amrex::EB2::lathe(polys), {AMREX_D_DECL(
                                static_cast<amrex::Real>(lenx * 0.5),
                                static_cast<amrex::Real>(leny * 0.5), 0.)});

  auto gshop = amrex::EB2::makeShop(pr);
  amrex::EB2::Build(gshop, geom, max_coarsening_level, max_coarsening_level);
}

void
ICEPistonBowl::build(
  const amrex::Geometry& geom, const int max_coarsening_level)
{
  amrex::RealArray center({AMREX_D_DECL(0.04 - 0.0125 - 0.02, 0.0, 0.0)});

  const amrex::Real radius = 0.02;

  const bool has_fluid_inside = false;
  amrex::EB2::SphereIF sf(radius, center, has_fluid_inside);

  amrex::EB2::CylinderIF cf1(
    0.04, 0.09, 0, {AMREX_D_DECL(0.045, 0.0, 0.0)}, true);
  amrex::EB2::CylinderIF cf2(
    0.04, 0.125, 0, {AMREX_D_DECL(-0.0125 - 0.02, 0.0, 0.0)}, false);
  amrex::EB2::CylinderIF cf3(
    0.03, 0.125, 0, {AMREX_D_DECL(-0.0125 - 0.02, 0.0, 0.0)}, false);
  auto pipe = amrex::EB2::makeDifference(cf2, cf3);
  amrex::EB2::CylinderIF cf4(
    0.03, 0.10, 0, {AMREX_D_DECL(-0.0125 - 0.02, 0.0, 0.0)}, false);

  amrex::RealArray center2({AMREX_D_DECL(0.0, 0.0, 0.0)});

  const amrex::Real radius2 = 0.09;

  const bool has_fluid_inside2 = true;
  amrex::EB2::SphereIF sf2(radius2, center2, has_fluid_inside2);

  auto polys = amrex::EB2::makeUnion(cf1, pipe, cf4, sf, sf2);

  auto gshop = amrex::EB2::makeShop(polys);
  amrex::EB2::Build(gshop, geom, max_coarsening_level, max_coarsening_level, 4);
}
void
ExtrudedTriangles::build(
  const amrex::Geometry& geom, const int max_coarsening_level)
{
  // setting some constants
  // the polygon is triangle
  // we can only do a maximum of 5 triangles (change if needed)
  const int npts_in_tri = 3;
  const int max_tri = 5;

  // number of user defined triangles
  int num_tri;

  amrex::ParmParse pp("extruded_triangles");
  amrex::Vector<amrex::Array<amrex::Real, AMREX_SPACEDIM>> alltri(
    npts_in_tri * max_tri);

  // initialize all triangles with some dummy values
  // that fall outside of the domain
  const amrex::Real* problo;
  const amrex::Real* probhi;
  amrex::Real maxlen;

  problo = geom.ProbLo();
  probhi = geom.ProbHi();

  maxlen = amrex::max<amrex::Real>(
    amrex::max<amrex::Real>(geom.ProbLength(0), geom.ProbLength(1)),
    geom.ProbLength(2));

  // setting all triangles to be waaay outside the domain initially
  for (int itri = 0; itri < max_tri; itri++) {
    alltri[npts_in_tri * itri + 0][0] = problo[0] + 100.0 * maxlen;
    alltri[npts_in_tri * itri + 0][1] = problo[1] - 100.0 * maxlen;
    alltri[npts_in_tri * itri + 0][2] = 0.0;

    alltri[npts_in_tri * itri + 1][0] = probhi[0] + 100.0 * maxlen;
    alltri[npts_in_tri * itri + 1][1] = problo[1] - 100.0 * maxlen;
    alltri[npts_in_tri * itri + 1][2] = 0.0;

    alltri[npts_in_tri * itri + 2][0] = probhi[0] + 100.0 * maxlen;
    alltri[npts_in_tri * itri + 2][1] = problo[1] + 100.0 * maxlen;
    alltri[npts_in_tri * itri + 2][2] = 0.0;
  }

  // get user defined number of triangles
  pp.get("num_tri", num_tri);

  for (int itri = 0; itri < num_tri; itri++) {
    amrex::Array<amrex::Real, AMREX_SPACEDIM> point{
      AMREX_D_DECL(0.0, 0.0, 0.0)};

    for (int ipt = 0; ipt < npts_in_tri; ipt++) {
      std::string pointstr =
        "tri_" + convertIntGG(itri) + "_point_" + convertIntGG(ipt);
      amrex::Vector<amrex::Real> vecpt;
      pp.getarr(pointstr.c_str(), vecpt, 0, AMREX_SPACEDIM);
      for (int dir = 0; dir < AMREX_SPACEDIM; dir++) {
        point[dir] = vecpt[dir];
      }
      alltri[npts_in_tri * itri + ipt] = point;
    }
  }

  // intersection of the 3 planes in a triangle for all triangles
  amrex::Vector<std::unique_ptr<amrex::EB2::IntersectionIF<
    amrex::EB2::PlaneIF, amrex::EB2::PlaneIF, amrex::EB2::PlaneIF>>>
    impfunc_triangles(max_tri);

  for (int itri = 0; itri < max_tri; itri++) {
    // make sure points are in anti clockwise direction to set the inside of
    // the triangle as solid phase correctly
    amrex::Array<amrex::Real, AMREX_SPACEDIM> norm0;
    amrex::Array<amrex::Real, AMREX_SPACEDIM> norm1;
    amrex::Array<amrex::Real, AMREX_SPACEDIM> norm2;

    amrex::Array<amrex::Real, AMREX_SPACEDIM> point0;
    amrex::Array<amrex::Real, AMREX_SPACEDIM> point1;
    amrex::Array<amrex::Real, AMREX_SPACEDIM> point2;

    point0 = alltri[npts_in_tri * itri + 0];
    point1 = alltri[npts_in_tri * itri + 1];
    point2 = alltri[npts_in_tri * itri + 2];

    norm0[0] = -(point1[1] - point0[1]);
    norm0[1] = (point1[0] - point0[0]);
    norm0[2] = 0.0;

    norm1[0] = -(point2[1] - point1[1]);
    norm1[1] = (point2[0] - point1[0]);
    norm1[2] = 0.0;

    norm2[0] = -(point0[1] - point2[1]);
    norm2[1] = (point0[0] - point2[0]);
    norm2[2] = 0.0;

    // normalize so that magnitude is 1
    amrex::Real norm = sqrt(norm0[0] * norm0[0] + norm0[1] * norm0[1]);
    norm0[0] = norm0[0] / norm;
    norm0[1] = norm0[1] / norm;

    // normalize so that magnitude is 1
    norm = sqrt(norm1[0] * norm1[0] + norm1[1] * norm1[1]);
    norm1[0] = norm1[0] / norm;
    norm1[1] = norm1[1] / norm;

    // normalize so that magnitude is 1
    norm = sqrt(norm2[0] * norm2[0] + norm2[1] * norm2[1]);
    norm2[0] = norm2[0] / norm;
    norm2[1] = norm2[1] / norm;

    amrex::EB2::PlaneIF plane0(point0, norm0);
    amrex::EB2::PlaneIF plane1(point1, norm1);
    amrex::EB2::PlaneIF plane2(point2, norm2);

    impfunc_triangles[itri] = std::make_unique<amrex::EB2::IntersectionIF<
      amrex::EB2::PlaneIF, amrex::EB2::PlaneIF, amrex::EB2::PlaneIF>>(

      plane0, plane1, plane2);
  }

  auto alltri_IF = amrex::EB2::makeUnion(
    *impfunc_triangles[0], *impfunc_triangles[1], *impfunc_triangles[2],
    *impfunc_triangles[3], *impfunc_triangles[4]);

  auto gshop = amrex::EB2::makeShop(alltri_IF);
  amrex::EB2::Build(gshop, geom, max_coarsening_level, max_coarsening_level);
}
void
PolygonRevolution::build(
  const amrex::Geometry& /*geom*/, const int /*max_coarsening_level*/)
{
  amrex::Print() << "polygon_revolution  geometry not currently supported. "
                    " combustor?\n";
  amrex::Abort();
}

void
QuarterCircle::build(
  const amrex::Geometry& geom, const int max_coarsening_level)
{
  amrex::Real r_inner = 1.0;
  amrex::Real r_outer = 2.0;
  amrex::ParmParse pp("eb2");
  pp.query("r_inner", r_inner);
  pp.query("r_outer", r_outer);

  amrex::EB2::CylinderIF inner(r_inner, 10, 2, {AMREX_D_DECL(0, 0, 0)}, false);
  amrex::EB2::CylinderIF outer(r_outer, 10, 2, {AMREX_D_DECL(0, 0, 0)}, true);

  auto polys = amrex::EB2::makeUnion(inner, outer);
  auto gshop = amrex::EB2::makeShop(polys);
  amrex::EB2::Build(
    gshop, geom, max_coarsening_level, max_coarsening_level, 4, false);
}

void
RotatedCylinder::build(
  const amrex::Geometry& geom, const int max_coarsening_level)
{
  bool inside = true;
  amrex::Real radius = 0.0002;
  int direction = 0;
  amrex::Vector<amrex::Real> centervec(3, 0.0);
  amrex::Real rotation = 0;
  int rotation_axe = 0;

  amrex::ParmParse pp("eb2");
  pp.query("cylinder_has_fluid_inside", inside);
  pp.query("cylinder_radius", radius);
  pp.query("cylinder_direction", direction);
  pp.query("cylinder_rotation", rotation);
  pp.query("cylinder_rotation_axe", rotation_axe);
  pp.getarr("cylinder_center", centervec, 0, 3);
  amrex::Array<amrex::Real, AMREX_SPACEDIM> center = {
    AMREX_D_DECL(centervec[0], centervec[1], centervec[2])};

  rotation = (rotation / 180.) * constants::PI();

  amrex::EB2::CylinderIF my_cyl(radius, direction, center, inside);

  auto my_cyl_rot = amrex::EB2::rotate(my_cyl, rotation, rotation_axe);
  auto gshop = amrex::EB2::makeShop(my_cyl_rot);
  amrex::EB2::Build(
    gshop, geom, max_coarsening_level, max_coarsening_level, 4, false);
}

void
RotatedBox::build(const amrex::Geometry& geom, const int max_coarsening_level)
{

  amrex::ParmParse pp("eb2");
  amrex::RealArray lo;
  pp.get("box_lo", lo);

  amrex::RealArray hi;
  pp.get("box_hi", hi);

  bool has_fluid_inside;
  pp.get("box_has_fluid_inside", has_fluid_inside);

  amrex::Real rotation = 0;
  int rotation_axe = 0;
  pp.query("box_rotation", rotation);
  pp.query("box_rotation_axe", rotation_axe);

  rotation = (rotation / 180.) * constants::PI();

  amrex::EB2::BoxIF bf(lo, hi, has_fluid_inside);

  auto bf_rot = amrex::EB2::rotate(bf, rotation, rotation_axe);
  auto gshop = amrex::EB2::makeShop(bf_rot);
  amrex::EB2::Build(
    gshop, geom, max_coarsening_level, max_coarsening_level, 4, false);
}

void TwoBranch::build (const amrex::Geometry& geom,
                       const int max_coarsening_level)
{
  using namespace amrex;
  using namespace amrex::EB2;

  ParmParse pp("geo");
  Real W=0.04, H=0.04, L=0.04, xs=0.30, xr=0.70;
  Real mid = 0.02;                 // center-wall thickness
  Real cL  = 0.00;                 // NEW: left connector length (retract from xs)
  Real cR  = 0.00;                 // NEW: right connector length (retract from xr)
  pp.query("W",W);  pp.query("H",H);  pp.query("L",L);
  pp.query("xs",xs); pp.query("xr",xr);
  pp.query("mid",mid);
  pp.query("cL",cL);
  pp.query("cR",cR);

  const RealBox& rb = geom.ProbDomain();
  const Real xlo = rb.lo(0), xhi = rb.hi(0);
  const Real ylo = rb.lo(1), yhi = rb.hi(1);
  const Real ymid = 0.5*(ylo+yhi);

  const Real dx = geom.CellSize(0);
  const Real dy = geom.CellSize(1);
  const Real h  = std::max(dx,dy);

  // sanity / resolvability
  xs  = std::min(std::max(xs, xlo+2*h),    xhi-2*h);
  xr  = std::min(std::max(xr, xs+6*h),     xhi-2*h);
  mid = std::min(std::max(mid, 4*h), std::max(W-4*h, 4*h+1e-12));

  // clamp connector lengths so wall still has positive span
  const Real max_pad = std::max(0.0, 0.5*(xr - xs) - 3*h);
  cL = std::min(std::max(cL, 0.0), max_pad);
  cR = std::min(std::max(cR, 0.0), max_pad);

  auto boxS = [] (Real x0, Real y0, Real x1, Real y1) {
    Array<Real,AMREX_SPACEDIM> lo{AMREX_D_DECL(std::min(x0,x1), std::min(y0,y1), 0.0)};
    Array<Real,AMREX_SPACEDIM> hi{AMREX_D_DECL(std::max(x0,x1), std::max(y0,y1), 0.0)};
    return BoxIF(lo, hi, /*has_fluid_inside=*/false); // SOLID
  };

  // bands
  const Real y_base_lo  = ymid - 0.5*W;
  const Real y_base_hi  = ymid + 0.5*W;
  const Real y_upper_lo = y_base_hi;
  const Real y_upper_hi = y_base_hi + H;
  const Real y_lower_lo = y_base_lo - L;
  const Real y_lower_hi = y_base_lo;

  // domain caps & outside-of-duct solids (unchanged)
  auto s_top    = boxS(xlo, y_upper_hi, xhi, yhi);
  auto s_bottom = boxS(xlo, ylo,       xhi, y_lower_lo);
  auto s_left_upper  = boxS(xlo, y_base_hi,  xs,  y_upper_hi);
  auto s_left_lower  = boxS(xlo, y_lower_lo, xs,  y_base_lo);
  auto s_right_upper = boxS(xr,  y_base_hi,  xhi, y_upper_hi);
  auto s_right_lower = boxS(xr,  y_lower_lo, xhi, y_base_lo);

  // mid-wall **retracted** by cL (left) and cR (right)
  const Real y_mid_lo = ymid - 0.5*mid;
  const Real y_mid_hi = ymid + 0.5*mid;
  const Real mw_x0 = xs + cL;
  const Real mw_x1 = xr - cR;
  auto s_mid_between  = boxS(mw_x0, y_mid_lo, mw_x1, y_mid_hi);

  // union
  auto u1    = EB2::makeUnion(s_top, s_bottom);
  auto u2    = EB2::makeUnion(u1, s_left_upper);
  auto u3    = EB2::makeUnion(u2, s_left_lower);
  auto u4    = EB2::makeUnion(u3, s_right_upper);
  auto u5    = EB2::makeUnion(u4, s_right_lower);
  auto walls = EB2::makeUnion(u5, s_mid_between);

  amrex::Print() << "[EB] TwoBranch connect: "
                 << "xs="<<xs<<" xr="<<xr<<" mid="<<mid
                 << " cL="<<cL<<" cR="<<cR
                 << " dx="<<dx<<" dy="<<dy << "\n";

  auto gshop = EB2::makeShop(walls);
  EB2::Build(gshop, geom, max_coarsening_level, max_coarsening_level, 128, false);
}

void
ThreeBranch::build (const amrex::Geometry& geom,
                    const int max_coarsening_level)
{
  amrex::Abort("HELLO from ThreeBranch");

  using namespace amrex;
  using namespace amrex::EB2;

  // ------------------- Controls (domain units) -------------------
  // We use a separate ParmParse prefix so you can tune independently
  // from your existing "geo.*" for TwoBranch.
  ParmParse pp("geo3");

  // Main two-branch parameters (same meaning as your TwoBranch)
  Real W  = 0.04;   // base duct height
  Real H  = 0.04;   // upper branch offset above base
  Real L  = 0.04;   // lower branch offset below base
  Real xs = 0.30;   // split x
  Real xr = 0.70;   // rejoin x
  Real mid = 0.02;  // center wall thickness (keeps upper/lower separated)
  Real cL  = 0.02;  // retract mid-wall from left end (opens mixing region)
  Real cR  = 0.02;  // retract mid-wall from right end

  // Third-branch (delay) parameters
  Real Zlen  = 0.20;  // horizontal length of the delay branch
  Real Hd    = 0.03;  // vertical drop from upper branch down to third branch
  Real h3    = 0.012; // vertical thickness (height) of the third branch
  Real wslot = 0.010; // width of the two vertical slots (drop/rise)
  Real x3s   = 0.38;  // x-location where the drop from upper begins

  pp.query("W",W);    pp.query("H",H);    pp.query("L",L);
  pp.query("xs",xs);  pp.query("xr",xr);  pp.query("mid",mid);
  pp.query("cL",cL);  pp.query("cR",cR);

  pp.query("Zlen",Zlen);
  pp.query("Hd",Hd);
  pp.query("h3",h3);
  pp.query("wslot",wslot);
  pp.query("x3s",x3s);

  const RealBox& rb = geom.ProbDomain();
  const Real xlo = rb.lo(0), xhi = rb.hi(0);
  const Real ylo = rb.lo(1), yhi = rb.hi(1);
  const Real ymid = 0.5*(ylo+yhi);

  const Real dx = geom.CellSize(0);
  const Real dy = geom.CellSize(1);
  const Real h  = std::max(dx,dy);

  // --------------- Sanity / resolvability clamps ----------------
  xs   = std::min(std::max(xs,  xlo+3*h),        xhi-3*h);
  xr   = std::min(std::max(xr,  xs + 8*h),       xhi-3*h);
  mid  = std::min(std::max(mid, 4*h),            std::max(W-4*h, 4*h+1e-12));
  cL   = std::min(std::max(cL,  0.0),            0.5*(xr-xs)-4*h);
  cR   = std::min(std::max(cR,  0.0),            0.5*(xr-xs)-4*h);

  // Delay branch geometry
  x3s  = std::min(std::max(x3s, xs + cL + 4*h),  xr - cR - 8*h);
  Zlen = std::max(Zlen, 8*h);
  Real x3e = std::min(x3s + Zlen, xr - cR - 4*h);
  wslot = std::max(wslot, 4*h);
  h3    = std::max(h3,    4*h);
  Hd    = std::max(Hd,    h3 + 4*h); // drop at least the branch thickness

  // -------------------- Helper: FLUID rectangles -----------------
  // We'll construct the FLUID geometry explicitly and take complement.
  auto boxF = [] (Real x0, Real y0, Real x1, Real y1) {
    Array<Real,AMREX_SPACEDIM> lo{AMREX_D_DECL(std::min(x0,x1),
                                               std::min(y0,y1), 0.0)};
    Array<Real,AMREX_SPACEDIM> hi{AMREX_D_DECL(std::max(x0,x1),
                                               std::max(y0,y1), 0.0)};
    return BoxIF(lo, hi, /*has_fluid_inside=*/true);
  };

  // Bands (like TwoBranch)
  const Real y_base_lo  = ymid - 0.5*W;
  const Real y_base_hi  = ymid + 0.5*W;
  const Real y_upper_lo = y_base_hi;
  const Real y_upper_hi = y_base_hi + H;
  const Real y_lower_lo = y_base_lo - L;
  const Real y_lower_hi = y_base_lo;

  // -------------------- FLUID: main ducts ------------------------
  auto left_main   = boxF(xlo, y_base_lo, xs,  y_base_hi);
  auto right_main  = boxF(xr,  y_base_lo, xhi, y_base_hi);

  // Upper & lower branch runs (between xs..xr)
  auto upper_run   = boxF(xs,  y_upper_lo, xr, y_upper_hi);
  auto lower_run   = boxF(xs,  y_lower_lo, xr, y_lower_hi);

  // Open connectors at ends (so upper & lower meet the main duct)
  auto up_conn_L   = boxF(xs,     y_base_hi,   xs + cL,  y_upper_hi);
  auto up_conn_R   = boxF(xr - cR, y_base_hi,  xr,       y_upper_hi);
  auto low_conn_L  = boxF(xs,     y_lower_lo,  xs + cL,  y_base_lo);
  auto low_conn_R  = boxF(xr - cR, y_lower_lo, xr,       y_base_lo);

  // -------------------- FLUID: third (delay) branch --------------
  // The third branch sits Hd below the upper branch floor, with thickness h3.
  const Real y3_hi = y_upper_lo - Hd;
  const Real y3_lo = y3_hi - h3;

  // Horizontal delay run
  auto third_run = boxF(x3s, y3_lo, x3e, y3_hi);

  // Vertical "drop" from upper branch down to third_run at x3s
  auto drop_L = boxF(x3s - 0.5*wslot, y3_hi, x3s + 0.5*wslot, y_upper_lo);

  // Vertical "rise" back up to upper branch at x3e
  auto rise_R = boxF(x3e - 0.5*wslot, y3_hi, x3e + 0.5*wslot, y_upper_lo);

  // -------------------- Union all FLUID pieces -------------------
  // Chain pairwise to keep AMReX happy (avoid deep variadics).
  auto f1 = makeUnion(left_main, right_main);
  auto f2 = makeUnion(f1, upper_run);
  auto f3 = makeUnion(f2, lower_run);
  auto f4 = makeUnion(f3, up_conn_L);
  auto f5 = makeUnion(f4, up_conn_R);
  auto f6 = makeUnion(f5, low_conn_L);
  auto f7 = makeUnion(f6, low_conn_R);
  auto f8 = makeUnion(f7, third_run);
  auto f9 = makeUnion(f8, drop_L);
  auto fluid = makeUnion(f9, rise_R);

  // -------------------- Make walls = complement ------------------
  auto walls = makeComplement(fluid);

  amrex::Print() << "[EB] ThreeBranch  "
                 << "W="<<W<<" H="<<H<<" L="<<L<<" xs="<<xs<<" xr="<<xr
                 << " mid="<<mid<<" cL="<<cL<<" cR="<<cR
                 << " | third: Zlen="<<Zlen<<" Hd="<<Hd<<" h3="<<h3
                 << " wslot="<<wslot<<" x3s="<<x3s
                 << " dx="<<dx<<" dy="<<dy << "\n";

  auto gshop = makeShop(walls);
  EB2::Build(gshop, geom, max_coarsening_level, max_coarsening_level, 128, false);
}




void
CheckpointFile::build(
  const amrex::Geometry& geom, const int max_coarsening_level)
{
  std::string chkfile = "chk_geom";
  amrex::ParmParse pp("eb2");
  pp.query("chkfile", chkfile);

  amrex::EB2::BuildFromChkptFile(chkfile, geom, 0, max_coarsening_level);
}
} // namespace pele::pelec
