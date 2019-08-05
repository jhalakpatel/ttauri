// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"
#include <string>

namespace TTauri::Config {

struct ASTMember : ASTExpression {
    ASTExpression *object;
    std::string name;

    ASTMember(Location location, ASTExpression *object, char *name) : ASTExpression(location), object(object), name(name) {
        free(name);
    }

    ~ASTMember() {
        delete object;
    }

    std::string string() const override {
        return object->string() + "." + name;
    }

    universal_value &executeLValue(ExecutionContext *context) const override {
        return object->executeLValue(context)[name];
    }

    universal_value &executeAssignment(ExecutionContext *context, universal_value other) const override {
        auto &lv = object->executeLValue(context)[name];
        lv = std::move(other);
        return lv;
    }
};

}
