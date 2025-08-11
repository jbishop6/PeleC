#pragma once
#include "BlackBoxFunction.H"
#include <memory>
#include <string>

// Minimal factory with a static Create(...) method.
// Always returns a basic no-op BlackBoxFunction instance.
class BlackBoxFunctionFactory {
public:
    static std::shared_ptr<BlackBoxFunction>
    Create(const std::string& /*name*/,
           const BlackBoxFunctionData* /*data*/ = nullptr)
    {
        return std::make_shared<BlackBoxFunction>();
    }
};
