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

  const amrex::RealBox& rb = geom.ProbDomain();
  const amrex::Real xlo = rb.lo(0), xhi = rb.hi(0);
  const amrex::Real ylo = rb.lo(1), yhi = rb.hi(1);
  const amrex::Real ymid = 0.5*(ylo+yhi);

  const amrex::Real dx = geom.CellSize(0);
  const amrex::Real dy = geom.CellSize(1);
  const amrex::Real h  = std::max(dx,dy);

  // sanity / resolvability
  xs  = std::min(std::max(xs, xlo+2*h),    xhi-2*h);
  xr  = std::min(std::max(xr, xs+6*h),     xhi-2*h);
  mid = std::min(std::max(mid, 4*h), std::max(W-4*h, 4*h+1e-12));

  // clamp connector lengths so wall still has positive span
  const amrex::Real max_pad = std::max(0.0, 0.5*(xr - xs) - 3*h);
  cL = std::min(std::max(cL, 0.0), max_pad);
  cR = std::min(std::max(cR, 0.0), max_pad);

  auto boxS = [] (Real x0, Real y0, Real x1, Real y1) {
    Array<Real,AMREX_SPACEDIM> lo{AMREX_D_DECL(std::min(x0,x1), std::min(y0,y1), 0.0)};
    Array<Real,AMREX_SPACEDIM> hi{AMREX_D_DECL(std::max(x0,x1), std::max(y0,y1), 0.0)};
    return BoxIF(lo, hi, /*has_fluid_inside=*/false); // SOLID
  };

  // bands
  const amrex::Real y_base_lo  = ymid - 0.5*W;
  const amrex::Real y_base_hi  = ymid + 0.5*W;
  const amrex::Real y_upper_lo = y_base_hi;
  const amrex::Real y_upper_hi = y_base_hi + H;
  const amrex::Real y_lower_lo = y_base_lo - L;
  const amrex::Real y_lower_hi = y_base_lo;

  // domain caps & outside-of-duct solids (unchanged)
  auto s_top    = boxS(xlo, y_upper_hi, xhi, yhi);
  auto s_bottom = boxS(xlo, ylo,       xhi, y_lower_lo);
  auto s_left_upper  = boxS(xlo, y_base_hi,  xs,  y_upper_hi);
  auto s_left_lower  = boxS(xlo, y_lower_lo, xs,  y_base_lo);
  auto s_right_upper = boxS(xr,  y_base_hi,  xhi, y_upper_hi);
  auto s_right_lower = boxS(xr,  y_lower_lo, xhi, y_base_lo);

  // mid-wall **retracted** by cL (left) and cR (right)
  const amrex::Real y_mid_lo = ymid - 0.5*mid;
  const amrex::Real y_mid_hi = ymid + 0.5*mid;
  const amrex::Real mw_x0 = xs + cL;
  const amrex::Real mw_x1 = xr - cR;
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
ThreeBranch::build(const amrex::Geometry& geom, const int max_coarsening_level)
{
  using namespace amrex;
  using namespace amrex::EB2;

  ParmParse pp("geo");
  Real W = 0.04, H = 0.04, L = 0.04, xs = 0.30, xr = 0.70;
  Real mid = 0.02;
  Real cL  = 0.00;
  Real cR  = 0.00;
  Real y_offset = 0.0;
  Real Z = 0.04;     // Height of third branch
  Real zW = 0.005;   // Wall thickness of third branch

  pp.query("W", W);  pp.query("H", H);  pp.query("L", L);
  pp.query("xs", xs); pp.query("xr", xr); pp.query("mid", mid);
  pp.query("cL", cL); pp.query("cR", cR); pp.query("y_offset", y_offset);
  pp.query("Z", Z);   pp.query("zW", zW);

  const RealBox& rb = geom.ProbDomain();
  const Real xlo = rb.lo(0), xhi = rb.hi(0);
  const Real ylo = rb.lo(1), yhi = rb.hi(1);
  const Real ymid = 0.5 * (ylo + yhi) + y_offset;

  const Real dx = geom.CellSize(0);
  const Real dy = geom.CellSize(1);
  const Real h  = std::max(dx, dy);

  xs  = std::min(std::max(xs, xlo + 2*h), xhi - 2*h);
  xr  = std::min(std::max(xr, xs + 6*h), xhi - 2*h);
  mid = std::min(std::max(mid, 4*h), std::max(W - 4*h, 4*h + 1e-12));

  const Real max_pad = std::max(0.0, 0.5*(xr - xs) - 3*h);
  cL = std::min(std::max(cL, 0.0), max_pad);
  cR = std::min(std::max(cR, 0.0), max_pad);

  auto boxS = [] (Real x0, Real y0, Real x1, Real y1) {
    Array<Real, AMREX_SPACEDIM> lo{AMREX_D_DECL(std::min(x0,x1), std::min(y0,y1), 0.0)};
    Array<Real, AMREX_SPACEDIM> hi{AMREX_D_DECL(std::max(x0,x1), std::max(y0,y1), 0.0)};
    return BoxIF(lo, hi, false);  // solid box
  };

  // Band heights
  const Real y_base_lo  = ymid - 0.5 * W;
  const Real y_base_hi  = ymid + 0.5 * W;
  const Real y_upper_lo = y_base_hi;
  const Real y_upper_hi = y_base_hi + H;
  const Real y_lower_lo = y_base_lo - L;
  const Real y_lower_hi = y_base_lo;

  // Mid-wall (center)
  const Real y_mid_lo = ymid - 0.5 * mid;
  const Real y_mid_hi = ymid + 0.5 * mid;
  const Real mw_x0 = xs + cL;
  const Real mw_x1 = xr - cR;

  // Base solids
  auto s_top          = boxS(xlo, y_upper_hi, xhi, yhi);
  auto s_bottom       = boxS(xlo, ylo,       xhi, y_lower_lo);
  auto s_left_upper   = boxS(xlo, y_base_hi, xs,  y_upper_hi);
  auto s_left_lower   = boxS(xlo, y_lower_lo, xs, y_base_lo);
  auto s_right_upper  = boxS(xr,  y_base_hi, xhi, y_upper_hi);
  auto s_right_lower  = boxS(xr,  y_lower_lo, xhi, y_base_lo);
  auto s_mid_between  = boxS(mw_x0, y_mid_lo, mw_x1, y_mid_hi);

  // Third (Z) branch at far-right bottom
  const Real z_x0 = xr;
  const Real z_x1 = xr + 2*zW;
  const Real z_y0 = y_lower_lo - Z;
  const Real z_y1 = y_lower_lo;

  auto s_z_left   = boxS(z_x0, z_y0, z_x0 + zW, z_y1);
  auto s_z_right  = boxS(z_x1 - zW, z_y0, z_x1, z_y1);
  auto s_z_bottom = boxS(z_x0, z_y0, z_x1, z_y0 + zW);

  // Combine all
  auto u1 = makeUnion(s_top, s_bottom);
  auto u2 = makeUnion(u1, s_left_upper);
  auto u3 = makeUnion(u2, s_left_lower);
  auto u4 = makeUnion(u3, s_right_upper);
  auto u5 = makeUnion(u4, s_right_lower);
  auto u6 = makeUnion(u5, s_mid_between);
  auto u7 = makeUnion(u6, s_z_left);
  auto u8 = makeUnion(u7, s_z_right);
  auto walls = makeUnion(u8, s_z_bottom);

  Print() << "Z-branch bounds: "
        << "z_x0=" << z_x0 << " to " << z_x0 + zW
        << ", z_y0=" << z_y0 << " to " << z_y1 << "\n";

  Print() << "Domain bounds: y = " << ylo << " to " << yhi << "\n";

  auto gshop = makeShop(walls);
  Build(gshop, geom, max_coarsening_level, max_coarsening_level, 128, false);
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
