#include <AMReX_EB2.H>
#include <AMReX_EB2_IF.H>
#include <AMReX_EB2_GeometryShop.H>
#include <AMReX_ParmParse.H>
#include "TwoBranchIF.H"   // your custom geometry class

using namespace amrex;
using namespace amrex::EB2;

void setupEBGeometry(const Geometry& geom, int required_level, int max_level)
{
    amrex::Print() << "[EB] setupEBGeometry: Initialize + Build\n";

    // Initialize Embedded Boundary
    EB2::Initialize();

    // Build the geometry using the makeGeometry() from TwoBranchIF.H
    auto shop = makeGeometry();

    // Construct the EB representation
    EB2::Build(shop, geom, required_level, max_level);

    amrex::Print() << "[EB] Geometry built successfully.\n";
}
