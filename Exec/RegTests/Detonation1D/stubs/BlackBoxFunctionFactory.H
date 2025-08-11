#pragma once
#include "BlackBoxFunction.H"

namespace pele {
namespace physics {

struct BlackBoxFunctionFactory {
    static BlackBoxFunctionPtr create(const BlackBoxFunctionData&, const std::string&) {
        return std::make_shared<BlackBoxFunction>();
    }
};

} // namespace physics
} // namespace pele
