#pragma once
#include <memory>
#include <string>

namespace pele {
namespace physics {

// Minimal payload type
struct BlackBoxFunctionData {};

// Minimal function object
class BlackBoxFunction {
public:
    virtual ~BlackBoxFunction() = default;
    virtual double eval(double) const { return 0.0; }
    virtual double eval(double,double,double,double) const { return 0.0; }
};

using BlackBoxFunctionPtr = std::shared_ptr<BlackBoxFunction>;

// ---- Key addition: specialize HostOnlyParm for BlackBoxFunctionData ----
// The primary template for HostOnlyParm<T> is defined in PelePhysics headers.
// By the time this stub is included, that primary template is already visible,
// so this explicit specialization is valid and will be used where needed.
template <typename T> struct HostOnlyParm;  // forward-declare template

template <>
struct HostOnlyParm<BlackBoxFunctionData> {
    BlackBoxFunctionData data;
    std::string          parm_parse_prefix;   // <-- what EosParams.H accesses
    BlackBoxFunctionData&       host_only_parm()       { return data; }
    BlackBoxFunctionData const& host_only_parm() const { return data; }
};

} // namespace physics
} // namespace pele
