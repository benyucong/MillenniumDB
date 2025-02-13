#pragma once

#include <memory>

#include "graph_models/object_id.h"
#include "graph_models/inliner.h"
#include "query/executor/binding_iter/binding_expr/binding_expr.h"

namespace MQL {
class BindingExprModulo : public BindingExpr {
public:
    std::unique_ptr<BindingExpr> lhs;
    std::unique_ptr<BindingExpr> rhs;

    BindingExprModulo(std::unique_ptr<BindingExpr> lhs, std::unique_ptr<BindingExpr> rhs) :
        lhs (std::move(lhs)),
        rhs (std::move(rhs)) { }

    ObjectId eval(const Binding& binding) override {
        // if (lhs.type == GraphObjectType::INT && rhs.type == GraphObjectType::INT
        //     && GraphObjectInterpreter::get<int64_t>(rhs) != 0)
        // {
        //     return GraphObjectFactory::make_int(GraphObjectInterpreter::get<int64_t>(lhs)
        //                                         % GraphObjectInterpreter::get<int64_t>(rhs));
        // }
        // return GraphObjectFactory::make_null();
        auto lhs_oid = lhs->eval(binding);
        auto rhs_oid = rhs->eval(binding);
        int64_t lhs_value;
        int64_t rhs_value;

        const auto lhs_type = lhs_oid.id & ObjectId::TYPE_MASK;
        const auto rhs_type = rhs_oid.id & ObjectId::TYPE_MASK;

        switch (lhs_type) {
        case ObjectId::MASK_NEGATIVE_INT: {
            int64_t i = (~lhs_oid.id) & 0x00FF'FFFF'FFFF'FFFFUL;
            lhs_value =  i*-1;
            break;
        }
        case ObjectId::MASK_POSITIVE_INT: {
            int64_t i = lhs_oid.id & 0x00FF'FFFF'FFFF'FFFFUL;
            lhs_value = i;
            break;
        }
        default:
            return ObjectId::get_null();
        }

        switch (rhs_type) {
        case ObjectId::MASK_NEGATIVE_INT: {
            int64_t i = (~rhs_oid.id) & 0x00FF'FFFF'FFFF'FFFFUL;
            rhs_value =  i*-1;
            break;
        }
        case ObjectId::MASK_POSITIVE_INT: {
            int64_t i = rhs_oid.id & 0x00FF'FFFF'FFFF'FFFFUL;
            rhs_value = i;
            break;
        }
        default:
            return ObjectId::get_null();
        }

        if (rhs_value == 0) {
            return ObjectId::get_null();
        }

        return ObjectId(Inliner::inline_int(lhs_value%rhs_value));
    }

    std::ostream& print_to_ostream(std::ostream& os) const override {
        os << '(' << *lhs << '%' << *rhs << ')';
        return os;
    }
};
} // namespace MQL
