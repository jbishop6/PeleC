#pragma once
#include <memory>
#include <string>
#include <vector>

// Minimal placeholder payload referenced by EosParams.H
struct BlackBoxFunctionData {
    std::string parm_parse_prefix;  // <- required by EosParams.H
};

class BlackBoxFunction {
public:
    virtual ~BlackBoxFunction() = default;
    virtual double eval(double) const { return 0.0; }
    virtual double eval(double, double, double, double) const { return 0.0; }
};

using BlackBoxFunctionPtr = std::shared_ptr<BlackBoxFunction>;
