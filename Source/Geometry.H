#ifndef GEOMETRY_H
#define GEOMETRY_H
#include "Factory.H"
#include "EB.H"
#include "Utilities.H"

namespace pele::pelec {

class Geometry : public physics::Factory<Geometry>
{
public:
  static std::string base_identifier() { return "Geometry"; }
  virtual void
  build(const amrex::Geometry& geom, const int max_coarsening_level) = 0;
  ~Geometry() override = default;
};

class FlatPlate : public Geometry::Register<FlatPlate>
{
public:
  static std::string identifier() { return "flat_plate"; }

  void
  build(const amrex::Geometry& geom, const int max_coarsening_level) override;
};

class Ramp : public Geometry::Register<Ramp>
{
public:
  static std::string identifier() { return "ramp"; }

  void
  build(const amrex::Geometry& geom, const int max_coarsening_level) override;
};

class Combustor : public Geometry::Register<Combustor>
{
public:
  static std::string identifier() { return "combustor"; }

  void
  build(const amrex::Geometry& geom, const int max_coarsening_level) override;
};

class ICEPistonBowl : public Geometry::Register<ICEPistonBowl>
{
public:
  static std::string identifier() { return "ICE_PistonBowl"; }

  void
  build(const amrex::Geometry& geom, const int max_coarsening_level) override;
};

class ExtrudedTriangles : public Geometry::Register<ExtrudedTriangles>
{
public:
  static std::string identifier() { return "extruded_triangles"; }

  void
  build(const amrex::Geometry& geom, const int max_coarsening_level) override;
};

class PolygonRevolution : public Geometry::Register<PolygonRevolution>
{
public:
  static std::string identifier() { return "polygon_revolution"; }

  void
  build(const amrex::Geometry& geom, const int max_coarsening_level) override;
};

class QuarterCircle : public Geometry::Register<QuarterCircle>
{
public:
  static std::string identifier() { return "quarter-circle"; }

  void
  build(const amrex::Geometry& geom, const int max_coarsening_level) override;
};

class RotatedCylinder : public Geometry::Register<RotatedCylinder>
{
public:
  static std::string identifier() { return "rotated_cylinder"; }

  void
  build(const amrex::Geometry& geom, const int max_coarsening_level) override;
};

class RotatedBox : public Geometry::Register<RotatedBox>
{
public:
  static std::string identifier() { return "rotated_box"; }

  void
  build(const amrex::Geometry& geom, const int max_coarsening_level) override;
};

class CheckpointFile : public Geometry::Register<CheckpointFile>
{
public:
  static std::string identifier() { return "chkfile"; }

  void
  build(const amrex::Geometry& geom, const int max_coarsening_level) override;
};

class TwoBranch : public Geometry::Register<TwoBranch>
{
public:
  static std::string identifier() { return "twobranch"; }
  void build(const amrex::Geometry& geom, const int max_coarsening_level) override;
};

void
ThreeBranch::build(const amrex::Geometry& geom, const int max_coarsening_level)
{
  using namespace amrex;
  using namespace amrex::EB2;

  Print() << "\n=== ULTRA-SIMPLE THREEBRANCH TEST ===\n";
  Print() << "Creating ONE solid box in center of domain\n";
  Print() << "If you don't see a solid square in the middle, something is fundamentally wrong\n";
  
  // Just create a single solid box - nothing else
  Array<Real, AMREX_SPACEDIM> lo{AMREX_D_DECL(0.3, 0.3, 0.0)};
  Array<Real, AMREX_SPACEDIM> hi{AMREX_D_DECL(0.7, 0.7, 0.0)};
  
  BoxIF solid_box(lo, hi, false);  // false = solid
  
  Print() << "Solid box coordinates: x=[0.3, 0.7], y=[0.3, 0.7]\n";
  Print() << "This should appear as a large square in the CENTER of your domain\n";
  Print() << "======================================\n\n";

  auto gshop = makeShop(solid_box);
  Build(gshop, geom, max_coarsening_level, max_coarsening_level, 128, false);
}



} // namespace pele::pelec
#endif
