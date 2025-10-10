#include <AMReX_EB2.H>
#include <AMReX_EB2_IF.H>
#include <AMReX_EB2_GeometryShop.H>
#include <AMReX_ParmParse.H>
#include "TwoBranchIF.H"

using namespace amrex;
using namespace amrex::EB2;

void setupEBGeometry(const Geometry& geom, int required_level, int max_level)
{
    amrex::Print() << "[EB] setupEBGeometry: Initialize + Build\n";

    EB2::Initialize();

    // makeGeometry() is defined in TwoBranchIF.H
    auto shop = makeGeometry();

    EB2::Build(shop, geom, required_level, max_level);

    amrex::Print() << "[EB] Geometry built successfully.\n";
}

REGISTER_EB2_SHAPE(TwoBranchIF, "twobranch")
