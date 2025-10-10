#ifndef TWOBRANCHIF_H_
#define TWOBRANCHIF_H_

#include <AMReX_Array.H>
#include <algorithm>
#include <cmath>

// A simple implicit function that defines two connected rectangular branches
class TwoBranchIF {
public:
    using Real = amrex::Real;
    using RealArray = amrex::GpuArray<Real, AMREX_SPACEDIM>;

    // Constructor: initialize all geometric parameters
    TwoBranchIF(Real xs, Real xr, Real W, Real H, Real mid, Real cL, Real cR)
        : m_xs(xs), m_xr(xr), m_W(W), m_H(H), m_mid(mid), m_cL(cL), m_cR(cR) {}

    // Implicit function operator — defines the EB geometry
    Real operator()(const RealArray& p) const {
        Real x = p[0];
        Real y = p[1];

        // ✅ Use member variables (m_...) for geometry
        Real left_x  = std::max(m_xs - x, x - (m_xs + m_cL));
        Real right_x = std::max((m_xr - m_cR) - x, x - m_xr);

        Real top_y = y - m_H;   // distance above top
        Real bot_y = -y;        // distance below bottom
        Real vert = std::max(top_y, bot_y);

        // Mid-section — small connecting gap in middle
        Real mid_y = std::max(std::abs(y - m_H / 2.0) - m_mid / 2.0, -m_mid / 2.0);

        // Return smallest distance (signed)
        return std::min({left_x, right_x, vert, mid_y});
    }

private:
    // Geometry parameters
    Real m_xs, m_xr;   // left/right x positions
    Real m_W, m_H;     // width and height of branches
    Real m_mid;        // middle connection height
    Real m_cL, m_cR;   // left/right connection lengths
};

#endif
