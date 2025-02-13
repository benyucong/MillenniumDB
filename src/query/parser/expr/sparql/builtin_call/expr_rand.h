#pragma once

#include <memory>

#include "query/parser/expr/expr.h"

namespace SPARQL {
class ExprRand : public Expr {
public:

    ExprRand() = default;

    void accept_visitor(ExprVisitor& visitor) override {
        visitor.visit(*this);
    }

    virtual std::unique_ptr<Expr> clone() const override {
        return std::make_unique<ExprRand>();
    }

    std::set<VarId> get_all_vars() const override {
        return {};
    }

    bool has_aggregation() const override {
        return false;
    }

    virtual std::ostream& print_to_ostream(std::ostream& os, int indent = 0) const override {
        return os << std::string(indent, ' ') << "RAND()";
    }
};
} // namespace SPARQL
