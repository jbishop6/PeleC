#ifndef TWOBRANCHIF_H_
#define TWOBRANCHIF_H_

#include <AMReX_Array.H>
#include <algorithm>
#include <cmath>

class TwoBranchIF {
public:
    using Real = amrex::Real;
    using RealArray = amrex::GpuArray<Real, AMREX_SPACEDIM>;

    // Constructor — initialize all member variables
    TwoBranchIF(Real xs, Real xr, Real W, Real H, Real mid, Real cL, Real cR)
        : m_xs(xs), m_xr(xr), m_W(W), m_H(H), m_mid(mid), m_cL(cL), m_cR(cR) {}

    // Implicit function operator
    Real operator()(const RealArray& p) const {
        Real x = p[0];
        Real y = p[1];

        // ✅ Use member variables (m_xs, m_cL, etc.), not undefined locals
        Real left_x  = std::max(m_xs - x, x - (m_xs + m_cL));
        Real right_x = std::max(m_xr - x, x - (m_xr + m_cR));
        Real mid_y   = std::abs(y - m_mid);

        return std::min({left_x, right_x, mid_y});
    }

private:
    // ✅ Only these are stored in the class — no duplicates
    Real m_xs, m_xr, m_W, m_H, m_mid, m_cL, m_cR;
};

#endif
