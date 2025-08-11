#pragma once
#include <memory>
#include <string>
#include <vector>

// Minimal placeholder payload referenced by EosParams.H
struct BlackBoxFunctionData {};

// A no-op interface matching what a "function" object might look like.
// We keep it super small so nothing links against real implementations.
class BlackBoxFunction {
public:
    virtual ~BlackBoxFunction() = default;

    // Generic evaluate hooks — return 0 by default
    virtual double eval(double /*x*/) const { return 0.0; }
    virtual double eval(double /*x*/, double /*y*/, double /*z*/, double /*t*/) const { return 0.0; }
};

// Handy alias some code may expect
using BlackBoxFunctionPtr = std::shared_ptr<BlackBoxFunction>;
