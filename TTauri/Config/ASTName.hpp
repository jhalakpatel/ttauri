
#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTName : ASTExpression {
    std::string name;

    ASTName(ASTLocation location, char *name) : ASTExpression(location), name(name) {
        free(name);
    }

    std::string str() const override {
        return name;
    }

    Value &executeLValue(ExecutionContext *context) const override {
        return context->currentObject()[name];
    } 

    Value &executeAssignment(ExecutionContext *context, Value other) const override {
        auto &lv = context->currentObject()[name];
        lv = std::move(other);
        return lv;
    }
};

}