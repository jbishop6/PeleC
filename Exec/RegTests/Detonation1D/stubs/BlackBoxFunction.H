#pragma once
#include <memory>
#include <string>

namespace pele {
namespace physics {

struct BlackBoxFunctionData {
    std::string parm_parse_prefix;  // required field
};

class BlackBoxFunction {
public:
    virtual ~BlackBoxFunction() = default;
    virtual double eval(double) const { return 0.0; }
    virtual double eval(double, double, double, double) const { return 0.0; }
};

using BlackBoxFunctionPtr = std::shared_ptr<BlackBoxFunction>;

} // namespace physics
} // namespace pele
