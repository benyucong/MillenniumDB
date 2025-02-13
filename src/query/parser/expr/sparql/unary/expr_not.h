#pragma once

#include <set>
#include <memory>

#include "query/parser/expr/expr.h"

namespace SPARQL {
class ExprNot : public Expr {
public:
    std::unique_ptr<Expr> expr;

    ExprNot(std::unique_ptr<Expr> expr) :
        expr (std::move(expr)) { }

    virtual std::unique_ptr<Expr> clone() const override {
        return std::make_unique<ExprNot>(expr->clone());
    }

    void accept_visitor(ExprVisitor& visitor) override {
        visitor.visit(*this);
    }

    std::set<VarId> get_all_vars() const override {
        return expr->get_all_vars();
    }

    bool has_aggregation() const override {
        return expr->has_aggregation();
    }

    virtual std::ostream& print_to_ostream(std::ostream& os, int indent = 0) const override {
        return os << std::string(indent, ' ') << "NOT(" << *expr << ')';
    }
};
} // namespace SPARQL
