#ifndef AMREX_TWOBRANCH_IF_H_
#define AMREX_TWOBRANCH_IF_H_

#include <AMReX_EB2_IF.H>
#include <AMReX_EB2_GeometryShop.H>
#include <AMReX_ParmParse.H>
#include <AMReX_REAL.H>
#include <cmath>

using namespace amrex;
using namespace amrex::EB2;

class TwoBranchIF
{
public:
    TwoBranchIF(amrex::Real xs, amrex::Real xr, amrex::Real W, amrex::Real H,
                amrex::Real mid, amrex::Real cL, amrex::Real cR)
        : m_xs(xs), m_xr(xr), m_W(W), m_H(H),
          m_mid(mid), m_cL(cL), m_cR(cR)
    {}

   AMREX_GPU_HOST_DEVICE
Real operator()(const amrex::RealArray& p) const
{
    // Always return positive (outside EB)
    return 1.0;
}


private:
    amrex::Real m_xs, m_xr, m_W, m_H, m_mid, m_cL, m_cR;
};

inline GeometryShop<TwoBranchIF> makeGeometry()
{
    amrex::ParmParse pp("geo");

    Real xs, xr, W, H, mid, cL, cR;
    pp.get("xs", xs);
    pp.get("xr", xr);
    pp.get("W", W);
    pp.get("H", H);
    pp.get("mid", mid);
    pp.get("cL", cL);
    pp.get("cR", cR);

    amrex::Print() << "[EB] TwoBranch connect: xs=" << xs
                   << " xr=" << xr
                   << " mid=" << mid
                   << " cL=" << cL
                   << " cR=" << cR << "\n";

    TwoBranchIF twobranch(xs, xr, W, H, mid, cL, cR);
    return GeometryShop<TwoBranchIF>(twobranch);
}

#endif
