#pragma once

#include "query/parser/expr/expr.h"

namespace MQL {
class ExprDivision : public Expr {
public:
    std::unique_ptr<Expr> lhs;
    std::unique_ptr<Expr> rhs;

    ExprDivision(std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs) :
        lhs (std::move(lhs)),
        rhs (std::move(rhs)) { }

    virtual std::unique_ptr<Expr> clone() const override {
        return std::make_unique<ExprDivision>(lhs->clone(), rhs->clone());
    }

    void accept_visitor(ExprVisitor& visitor) override {
        visitor.visit(*this);
    }

    bool has_aggregation() const override {
        return lhs->has_aggregation() or rhs->has_aggregation();
    }

    std::set<VarId> get_all_vars() const override {
        std::set<VarId> res = lhs->get_all_vars();
        auto rhs_vars = rhs->get_all_vars();
        res.insert(rhs_vars.begin(), rhs_vars.end());
        return res;
    }

    std::ostream& print_to_ostream(std::ostream& os, int indent = 0) const override {
        return os << std::string(indent, ' ') << '(' <<*lhs << " / " << *rhs <<  ')' ;
    }
};
} // namespace MQL
