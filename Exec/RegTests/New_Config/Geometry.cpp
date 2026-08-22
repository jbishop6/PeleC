#warning Compiling custom Geometry.cpp from pelec-detonation!

#include "Geometry.H"

#include <AMReX_EB2.H>
#include <AMReX_EB2_IF.H>
#include <AMReX_EB2_IF_Union.H>
#include <AMReX_EB2_IF_Intersection.H>
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

  AMREX_D_TERM(
    maxlen = geom.ProbLength(0);
    , maxlen = amrex::max<amrex::Real>(maxlen, geom.ProbLength(1));
    , maxlen = amrex::max<amrex::Real>(maxlen, geom.ProbLength(2)););

  // setting all triangles to be waaay outside the domain initially
  for (int itri = 0; itri < max_tri; itri++) {
    AMREX_D_TERM(
      alltri[npts_in_tri * itri + 0][0] = problo[0] + 100.0 * maxlen;
      , alltri[npts_in_tri * itri + 0][1] = problo[1] - 100.0 * maxlen;
      , alltri[npts_in_tri * itri + 0][2] = 0.0;);

    AMREX_D_TERM(
      alltri[npts_in_tri * itri + 1][0] = probhi[0] + 100.0 * maxlen;
      , alltri[npts_in_tri * itri + 1][1] = problo[1] - 100.0 * maxlen;
      , alltri[npts_in_tri * itri + 1][2] = 0.0;);

    AMREX_D_TERM(
      alltri[npts_in_tri * itri + 2][0] = probhi[0] + 100.0 * maxlen;
      , alltri[npts_in_tri * itri + 2][1] = problo[1] + 100.0 * maxlen;
      , alltri[npts_in_tri * itri + 2][2] = 0.0;);
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
      pp.getarr(pointstr, vecpt, 0, AMREX_SPACEDIM);
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

    AMREX_D_TERM(
      norm0[0] = -(point1[1] - point0[1]);, norm0[1] = (point1[0] - point0[0]);
      , norm0[2] = 0.0;);

    AMREX_D_TERM(
      norm1[0] = -(point2[1] - point1[1]);, norm1[1] = (point2[0] - point1[0]);
      , norm1[2] = 0.0;);

    AMREX_D_TERM(
      norm2[0] = -(point0[1] - point2[1]);, norm2[1] = (point0[0] - point2[0]);
      , norm2[2] = 0.0;);

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

void TwoBranch::build(const amrex::Geometry& geom,
                      const int max_coarsening_level)
{
  using namespace amrex;
  using namespace amrex::EB2;

  ParmParse pp("geo");

  // Same interface as ThreeBranch
  Real W = 0.04, H = 0.04, L = 0.04, xs = 0.30;
  Real X = 0.40;
  Real mid = 0.02;
  Real cL  = 0.00;
  Real cR  = 0.00;
  Real y_offset = 0.0;
  Real Z = 0.08;  // kept for symmetry with ThreeBranch, but not used here

  pp.query("W", W);  
  pp.query("H", H);   // separator thickness
  pp.query("L", L);   // channel height
  pp.query("xs", xs); 
  pp.query("X", X);
  pp.query("mid", mid);
  pp.query("cL", cL); 
  pp.query("cR", cR); 
  pp.query("y_offset", y_offset);
  pp.query("Z", Z);   // ignored in TwoBranch

  const RealBox& rb = geom.ProbDomain();

  const Real xlo = rb.lo(0), xhi = rb.hi(0);
  const Real ylo = rb.lo(1), yhi = rb.hi(1);
  const Real ymid = 0.5 * (ylo + yhi) + y_offset;

  const Real dx = geom.CellSize(0);
  const Real dy = geom.CellSize(1);
  const Real h  = std::max(dx, dy);

  xs = std::min(std::max(xs, xlo + 2*h), xhi - 2*h);
  Real xr = xs + X;
  
  // Same clamp on mid as in ThreeBranch
  mid = std::min(std::max(mid, 4*h), std::max(W - 4*h, 4*h + 1e-12));

  const Real max_pad = std::max(0.0, 0.5*(xr - xs) - 3*h);
  cL = std::min(std::max(cL, 0.0), max_pad);
  cR = std::min(std::max(cR, 0.0), max_pad);

  auto boxS = [] (Real x0, Real y0, Real x1, Real y1) {
    Array<Real, AMREX_SPACEDIM> lo{
      AMREX_D_DECL(std::min(x0,x1), std::min(y0,y1), 0.0)};
    Array<Real, AMREX_SPACEDIM> hi{
      AMREX_D_DECL(std::max(x0,x1), std::max(y0,y1), 0.0)};
    return BoxIF(lo, hi, false);  // solid
  };

  // ---- Vertical layout (identical to ThreeBranch) ----
  const Real y_base_lo  = ymid - 0.5 * H;  // separator bottom
  const Real y_base_hi  = ymid + 0.5 * H;  // separator top
  const Real y_upper_lo = y_base_hi;
  const Real y_upper_hi = y_upper_lo + L;
  const Real y_lower_hi = y_base_lo;
  const Real y_lower_lo = y_lower_hi - L;

  const Real y_mid_lo = ymid - 0.5 * mid;
  const Real y_mid_hi = ymid + 0.5 * mid;

  const Real mw_x0 = xs + cL;
  const Real mw_x1 = xr - cR;

  // Third-branch extents (computed the same, but we WON'T open it)
  const Real z_x_left  = mw_x1;
  const Real z_x_right = mw_x1 + W; 
  const Real z_y_top   = y_lower_lo;
  const Real z_y_bottom = std::max(z_y_top - Z, ylo + 2*h);

  Print() << "\n=== TWO-BRANCH GEOMETRY (ThreeBranch core, NO third branch) ===\n";
  Print() << "Separator thickness H: " << H << "\n";
  Print() << "Channel height L: " << L << " (constant)\n";
  Print() << "Total system height: " << (2*L + H) << "\n";
  Print() << "Upper channel: y=[" << y_upper_lo << ", " << y_upper_hi
          << "], height=" << (y_upper_hi - y_upper_lo) << "\n";
  Print() << "Separator: y=[" << y_base_lo << ", " << y_base_hi
          << "], thickness=" << H << "\n";
  Print() << "Lower channel: y=[" << y_lower_lo << ", " << y_lower_hi
          << "], height=" << (y_lower_hi - y_lower_lo) << "\n";
  Print() << "xs=" << xs << ", xr=" << xr << ", X=" << X << "\n";
  Print() << "cL=" << cL << ", cR=" << cR << "\n";
  Print() << "Blue separator: x=[" << mw_x0 << ", " << mw_x1 << "]\n";
  Print() << "Third-branch region (SEALED): x=[" << z_x_left << ", " << z_x_right 
          << "], y=[" << z_y_bottom << ", " << z_y_top << "]\n";
  Print() << "===============================================================\n\n";

  // ---- Walls: EXACT same structure, but fill the bottom gap ----

  // Top boundary wall
  auto s_top = boxS(xlo, y_upper_hi, xhi, yhi);

  // Bottom walls: ThreeBranch splits into left / under-gap / right.
  // Here we *fill* the under-gap up to y_lower_lo, so the third branch is solid.
  auto s_bottom_left  = boxS(xlo,      ylo, mw_x1,      y_lower_lo);
  auto s_bottom_mid   = boxS(mw_x1,    ylo, z_x_right,  y_lower_lo); // <- filled, no opening
  auto s_bottom_right = boxS(z_x_right,ylo, xhi,        y_lower_lo);

  // Two-branch side walls (same as ThreeBranch)
  auto s_left_upper   = boxS(xlo, y_base_hi,  xs,        y_upper_hi);
  auto s_left_lower   = boxS(xlo, y_lower_lo, xs,        y_base_lo);
  auto s_right_upper  = boxS(xr,  y_base_hi,  xhi,       y_upper_hi);
  auto s_right_lower  = boxS(z_x_right, y_lower_lo, xhi, y_base_lo);

  // Mid connector between upper & lower channel
  auto s_mid_between  = boxS(mw_x0, y_mid_lo, mw_x1,    y_mid_hi);

  // Union everything in the same order as ThreeBranch
  auto u1 = makeUnion(s_top,         s_bottom_left);
  auto u2 = makeUnion(u1,            s_bottom_mid);
  auto u3 = makeUnion(u2,            s_bottom_right);
  auto u4 = makeUnion(u3,            s_left_upper);
  auto u5 = makeUnion(u4,            s_left_lower);
  auto u6 = makeUnion(u5,            s_right_upper);
  auto u7 = makeUnion(u6,            s_right_lower);
  auto walls = makeUnion(u7,         s_mid_between);

  auto gshop = makeShop(walls);
  Build(gshop, geom, max_coarsening_level, max_coarsening_level, 128, false);
}

void ThreeBranch::build(const amrex::Geometry& geom, const int max_coarsening_level)
{
  using namespace amrex;
  using namespace amrex::EB2;

  ParmParse pp("geo");

  Real W = 0.04, H = 0.04, L = 0.04, xs = 0.30;
  Real X = 0.40;
  Real mid = 0.02;
  Real cL  = 0.00;
  Real cR  = 0.00;
  Real y_offset = 0.0;
  Real Z = 0.08;

  pp.query("W", W);  
  pp.query("H", H);  // Controls ONLY blue separator thickness
  pp.query("L", L);  // Controls channel heights (stays constant)
  pp.query("xs", xs); 
  pp.query("X", X);
  pp.query("mid", mid);
  pp.query("cL", cL); 
  pp.query("cR", cR); 
  pp.query("y_offset", y_offset);
  pp.query("Z", Z);

  const RealBox& rb = geom.ProbDomain();

  const Real xlo = rb.lo(0), xhi = rb.hi(0);
  const Real ylo = rb.lo(1), yhi = rb.hi(1);
  const Real ymid = 0.5 * (ylo + yhi) + y_offset;

  const Real dx = geom.CellSize(0);
  const Real dy = geom.CellSize(1);
  const Real h  = std::max(dx, dy);

  xs = std::min(std::max(xs, xlo + 2*h), xhi - 2*h);
  
  Real xr = xs + X;
  
  mid = std::min(std::max(mid, 4*h), std::max(W - 4*h, 4*h + 1e-12));

  const Real max_pad = std::max(0.0, 0.5*(xr - xs) - 3*h);
  cL = std::min(std::max(cL, 0.0), max_pad);
  cR = std::min(std::max(cR, 0.0), max_pad);

  auto boxS = [] (Real x0, Real y0, Real x1, Real y1) {
    Array<Real, AMREX_SPACEDIM> lo{AMREX_D_DECL(std::min(x0,x1), std::min(y0,y1), 0.0)};
    Array<Real, AMREX_SPACEDIM> hi{AMREX_D_DECL(std::max(x0,x1), std::max(y0,y1), 0.0)};
    return BoxIF(lo, hi, false);
  };

  // CORRECTED: H controls separator, L controls channel heights independently
  const Real y_base_lo  = ymid - 0.5 * H;  // Bottom of separator
  const Real y_base_hi  = ymid + 0.5 * H;  // Top of separator
  const Real y_upper_lo = y_base_hi;       // Upper channel starts at top of separator
  const Real y_upper_hi = y_upper_lo + L;  // Upper channel height = L (independent of H)
  const Real y_lower_hi = y_base_lo;       // Lower channel ends at bottom of separator
  const Real y_lower_lo = y_lower_hi - L;  // Lower channel height = L (independent of H)

  const Real y_mid_lo = ymid - 0.5 * mid;
  const Real y_mid_hi = ymid + 0.5 * mid;

  const Real mw_x0 = xs + cL;
  const Real mw_x1 = xr - cR;

  // Third branch positioned at RIGHT EDGE of blue separator
  const Real z_x_left = mw_x1;
  const Real z_x_right = mw_x1 + W; 
  const Real z_y_top = y_lower_lo;
  const Real z_y_bottom = std::max(z_y_top - Z, ylo + 2*h);

  Print() << "\n=== THREE-BRANCH GEOMETRY ===\n";
  Print() << "Separator thickness H: " << H << "\n";
  Print() << "Channel height L: " << L << " (constant)\n";
  Print() << "Total system height: " << (2*L + H) << "\n";
  Print() << "Upper channel: y=[" << y_upper_lo << ", " << y_upper_hi << "], height=" << (y_upper_hi - y_upper_lo) << "\n";
  Print() << "Separator: y=[" << y_base_lo << ", " << y_base_hi << "], thickness=" << H << "\n";
  Print() << "Lower channel: y=[" << y_lower_lo << ", " << y_lower_hi << "], height=" << (y_lower_hi - y_lower_lo) << "\n";
  Print() << "xs=" << xs << ", xr=" << xr << ", X=" << X << "\n";
  Print() << "cL=" << cL << ", cR=" << cR << "\n";
  Print() << "Blue separator: x=[" << mw_x0 << ", " << mw_x1 << "]\n";
  Print() << "Third branch: x=[" << z_x_left << ", " << z_x_right 
          << "], y=[" << z_y_bottom << ", " << z_y_top << "]\n";
  Print() << "=============================\n\n";

  // Top boundary wall
  auto s_top = boxS(xlo, y_upper_hi, xhi, yhi);

  // Bottom walls - gap starts at mw_x1
  auto s_bottom_left  = boxS(xlo, ylo, mw_x1, y_lower_lo);
  auto s_bottom_under = boxS(mw_x1, ylo, z_x_right, z_y_bottom);
  auto s_bottom_right = boxS(z_x_right, ylo, xhi, y_lower_lo);

  // Two-branch system walls
  auto s_left_upper   = boxS(xlo, y_base_hi, xs, y_upper_hi);
  auto s_left_lower   = boxS(xlo, y_lower_lo, xs, y_base_lo);
  auto s_right_upper  = boxS(xr, y_base_hi, xhi, y_upper_hi);
  auto s_right_lower  = boxS(z_x_right, y_lower_lo, xhi, y_base_lo);

  auto s_mid_between  = boxS(mw_x0, y_mid_lo, mw_x1, y_mid_hi);

  // Union everything
  auto u1 = makeUnion(s_top, s_bottom_left);
  auto u2 = makeUnion(u1, s_bottom_under);
  auto u3 = makeUnion(u2, s_bottom_right);
  auto u4 = makeUnion(u3, s_left_upper);
  auto u5 = makeUnion(u4, s_left_lower);
  auto u6 = makeUnion(u5, s_right_upper);
  auto u7 = makeUnion(u6, s_right_lower);
  auto walls = makeUnion(u7, s_mid_between);

  Print() << "[EB] ThreeBranch: H controls separator only, L stays constant\n";

  auto gshop = makeShop(walls);
  Build(gshop, geom, max_coarsening_level, max_coarsening_level, 128, false);
}

void
TwoBranch_NewConfig::build(
    const amrex::Geometry& geom,
    const int max_coarsening_level)
{
    using namespace amrex;
    using namespace amrex::EB2;

    ParmParse pp("twobranch_newconfig");

    // ============================================================
    // ONLY USER-ADJUSTABLE PARAMETERS
    // ============================================================

    Real X = 0.100;   // total horizontal length
    Real Y = 0.050;   // inner lower horizontal length
    Real L = 0.015;   // lower vertical offset
    Real H = 0.030;   // upper vertical offset

    pp.query("X", X);
    pp.query("Y", Y);
    pp.query("L", L);
    pp.query("H", H);

    // ============================================================
    // DOMAIN
    // ============================================================

    const RealBox& rb = geom.ProbDomain();

    const Real xlo = rb.lo(0);
    const Real xhi = rb.hi(0);
    const Real ylo = rb.lo(1);
    const Real yhi = rb.hi(1);

    // ============================================================
    // AUTOMATIC / DERIVED DIMENSIONS
    // ============================================================

    // Channel thickness scales automatically with L and H.
    const Real channel_t = 0.20 * std::min(L, H);

    // Horizontal placement:
    // center an X-long geometry in the computational domain.
    const Real x1 = 0.5 * (xlo + xhi - X);
    const Real x5 = x1 + X;

    // Points 2 and 3 are automatically inset from the ends.
    const Real outer_inset = 0.15 * X;

    const Real x2 = x1 + outer_inset;

    // Vertical placement:
    // total timing-model height is L + H.
    const Real total_height = L + H;

    const Real y2 =
        0.5 * (ylo + yhi - total_height);

    const Real y_shelf = y2 + L;
    const Real y5      = y_shelf + H;

    // ============================================================
    // INNER GEOMETRY
    // ============================================================

    // Left inner wall sits one channel thickness inward.
    const Real xi_left = x2 + channel_t;

    // Y directly controls Point 4.
    const Real x4 = xi_left + Y;
        // Point 3 follows Point 4 so the lower-right turn
    // keeps a consistent width as Y changes.
    const Real right_turn_width = 2.0 * channel_t;
    const Real x3 = x4 + right_turn_width;

     if (x3 >= x5 - channel_t) {
      Print() << "\nERROR: Geometry does not fit in the domain.\n";
      Print() << "x3 = " << x3 << "\n";
      Print() << "x5 = " << x5 << "\n";
      Abort("TwoBranch_NewConfig: increase X or decrease Y");
      }

    const Real xi_right = x5 - 0.5 * channel_t;
  
    // Vertical inner contour.
    const Real yi_low =
        y2 + channel_t;

    const Real yi_top =
        y5 - channel_t;

    // Step is automatically one channel thickness below the shelf.
    const Real yi_step =
        y_shelf - channel_t;

    // ============================================================
    // HELPER: FLUID BOX
    // ============================================================

    auto fluidBox =
      [] (Real xa, Real ya, Real xb, Real yb)
    {
        Array<Real, AMREX_SPACEDIM> lo{
            AMREX_D_DECL(
                std::min(xa, xb),
                std::min(ya, yb),
                0.0)};

        Array<Real, AMREX_SPACEDIM> hi{
            AMREX_D_DECL(
                std::max(xa, xb),
                std::max(ya, yb),
                0.0)};

        // true = fluid inside this box
        return BoxIF(lo, hi, true);
    };

    // ============================================================
    // FLUID CHANNEL
    // ============================================================

    // -------------------------------
    // Upper horizontal passage
    // -------------------------------

    auto upper_channel =
        fluidBox(
            x1,
            yi_top,
            x5,
            y5);

    // -------------------------------
    // Left connector
    // -------------------------------

    auto left_connector =
        fluidBox(
            x2,
            y2,
            xi_left,
            y5);

    // -------------------------------
    // Lower horizontal passage
    // -------------------------------

    auto lower_channel =
        fluidBox(
            x2,
            y2,
            x4,
            yi_low);

    // -------------------------------
    // Point-4 / lower-right region
    // -------------------------------

      // ------------------------------------------------------------
  // Right-hand connection
  //
  // Intentionally overlap the three pieces by channel_t so the
  // EB grid sees one continuous fluid passage.
  // ------------------------------------------------------------
  
  // Lower branch turns upward toward the shelf
  
  // Keep the right vertical leg about one channel thickness wide
const Real x_right_leg_left = x3 - channel_t;

// Horizontal bridge from the lower channel into the right leg
// Lower vertical turn from the lower channel
const Real connector_overlap = 0.5 * channel_t;
  
  auto right_lower =
    fluidBox(
        x4 - connector_overlap,
        y2,
        x4 + channel_t - connector_overlap,
        yi_step);
  
  // Horizontal step at Point 4
  auto right_step =
      fluidBox(
          x4 - 0.5 * channel_t,
          yi_step,
          xi_right,
          y_shelf);
  
  // Skinny upper-right vertical passage
  auto right_upper =
      fluidBox(
          xi_right,
          y_shelf,
          x5,
          y5);

    // ============================================================
    // UNION OF ALL FLUID REGIONS
    // ============================================================
    
     auto f1 = makeIntersection(
        upper_channel,
        left_connector);
    
    auto f2 = makeIntersection(
        f1,
        lower_channel);
    
    auto f3 = makeIntersection(
        f2,
        right_lower);
    
    auto f4 = makeIntersection(
        f3,
        right_step);
    
    auto fluid_geometry = makeIntersection(
        f4,
        right_upper);
    // ============================================================
    // DEBUG OUTPUT
    // ============================================================

    Print() << "\n========================================\n";
    Print() << " TWO-BRANCH NEW CONFIG\n";
    Print() << "========================================\n";

    Print() << "INPUT PARAMETERS:\n";
    Print() << "X = " << X << "\n";
    Print() << "Y = " << Y << "\n";
    Print() << "L = " << L << "\n";
    Print() << "H = " << H << "\n";

    Print() << "\nDERIVED PARAMETERS:\n";
    Print() << "channel_t   = " << channel_t << "\n";

    Print() << "\nOUTER POINTS:\n";
    Print() << "P1 = (" << x1 << ", " << y5 << ")\n";
    Print() << "P2 = (" << x2 << ", " << y2 << ")\n";
    Print() << "P3 = (" << x3 << ", " << y2 << ")\n";
    Print() << "P5 = (" << x5 << ", " << y5 << ")\n";

    Print() << "\nINNER GEOMETRY:\n";
    Print() << "xi_left  = " << xi_left << "\n";
    Print() << "x4       = " << x4 << "\n";
    Print() << "xi_right = " << xi_right << "\n";

    Print() << "yi_low   = " << yi_low << "\n";
    Print() << "yi_step  = " << yi_step << "\n";
    Print() << "yi_top   = " << yi_top << "\n";

    Print() << "========================================\n\n";

    // ============================================================
    // BUILD EB
    // ============================================================

    auto gshop = makeShop(fluid_geometry);

    Build(
        gshop,
        geom,
        max_coarsening_level,
        max_coarsening_level,
        128,
        false);
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
