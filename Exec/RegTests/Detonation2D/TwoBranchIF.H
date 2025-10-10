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
        : m_xs(xs), m_xr(xr), m_W(W), m_H(H), m_mid(mid), m_cL(cL), m_cR(cR)
    {}

    AMREX_GPU_HOST_DEVICE
    Real operator() (const amrex::RealArray& p) const
    {
        Real x = p[0];
        Real y = p[1];

        // Left branch: from xs to xs + cL
        Real left_x = std::max(xs - x, x - (xs + cL));
        Real left_y = std::max(y - H, -y);
        Real d_left = std::max(left_x, left_y);

        // Right branch: from xr - cR to xr
        Real right_x = std::max((xr - cR) - x, x - xr);
        Real right_y = std::max(y - H, -y);
        Real d_right = std::max(right_x, right_y);

        // Mid wall: from xs + cL to xr - cR, has a small channel "mid"
        Real mid_x = std::max((xs + cL) - x, x - (xr - cR));
        Real mid_y = std::max(std::abs(y - H/2.0) - mid/2.0, -mid/2.0);
        Real d_mid = std::max(mid_x, mid_y);

        // Combine using intersection
        return std::min(std::min(d_left, d_right), d_mid);
    }

private:
    amrex::Real m_xs, m_xr, m_W, m_H, m_mid, m_cL, m_cR;
};

// Geometry registration function
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

    amrex::Print() << "[EB] TwoBranch connect: xs=" << xs << " xr=" << xr
                   << " mid=" << mid << " cL=" << cL << " cR=" << cR << "\n";

    TwoBranchIF twobranch(xs, xr, W, H, mid, cL, cR);
    return GeometryShop<TwoBranchIF>(twobranch);
}

#endif
