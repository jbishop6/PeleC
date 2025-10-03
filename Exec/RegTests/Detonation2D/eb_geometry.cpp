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

void setupEBGeometry()
{
    const Geometry& geom = PeleC::top()->Geom(0);
    Initialize_EB2(geom, 0, 0);  // or inline the logic directly
}
