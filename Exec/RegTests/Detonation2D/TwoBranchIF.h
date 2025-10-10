#ifndef TWOBRANCHIF_H_
#define TWOBRANCHIF_H_

#include <AMReX_Array.H>
#include <algorithm>
#include <cmath>

class TwoBranchIF {
public:
    using Real = amrex::Real;
    using RealArray = amrex::GpuArray<Real, AMREX_SPACEDIM>;

    // Constructor
    TwoBranchIF(Real xs_in, Real xr_in, Real mid_in, Real cL_in, Real cR_in)
        : xs(xs_in), xr(xr_in), mid(mid_in), cL(cL_in), cR(cR_in) {}

    // Implicit function
    Real operator()(const RealArray& p) const {
        Real x = p[0];
        Real y = p[1];

        Real left_x  = std::max(xs - x, x - (xs + cL));
        Real right_x = std::max(xr - x, x - (xr + cR));
        Real mid_y   = std::abs(y - mid);

        return std::min({left_x, right_x, mid_y});
    }

private:
    Real xs, xr, mid, cL, cR;  // ✅ These are the missing variables
};

#endif
