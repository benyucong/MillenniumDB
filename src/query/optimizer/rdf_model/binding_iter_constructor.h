#pragma once

#include <map>
#include <set>
#include <memory>

#include "query/executor/binding_iter.h"
#include "query/exceptions.h"
#include "query/id.h"
#include "query/executor/binding_iter/aggregation/agg.h"
#include "query/executor/binding_iter/order_by_id.h"
#include "query/executor/binding_iter/projection_order_exprs.h"
#include "query/parser/op/op_visitor.h"
#include "query/parser/op/sparql/op_construct.h"
#include "query/parser/op/sparql/ops.h"

namespace SPARQL {

struct JoinVars {
    // Safe variables from parent before evaluating lhs or rhs.
    std::set<VarId> parent_safe_vars;

    // Common scope variables of lhs and rhs.
    std::set<VarId> common_vars;

    // common_vars without parent_safe_vars.
    // join_vars can be divided into safe and unsafe join_vars.
    std::set<VarId> join_vars;

    // join_vars that are safe (safe in lhs and fixable in rhs).
    std::set<VarId> safe_join_vars;

    // join_vars that are not safe (not safe in lhs, or not fixable in rhs, or both).
    std::set<VarId> unsafe_join_vars;

    // variables that can be fixed when evaluating lhs.
    std::set<VarId> lhs_fixable_vars;

    // variables that can be fixed when evaluating rhs
    std::set<VarId> rhs_fixable_vars;

    // variables in lhs scope variables that are not in rhs or in parent_safe_vars.
    std::set<VarId> lhs_only_vars;

    // variables in rhs scope variables that are not in lhs or in parent_safe_vars.
    std::set<VarId> rhs_only_vars;

    // variables that are safe after evaluating lhs.
    std::set<VarId> after_left_safe_vars;

    // variables that are possibly assigned after evaluating lhs.
    std::set<VarId> after_left_possible_assigned_vars;
};

class BindingIterConstructor : public OpVisitor {
public:
    BindingIterConstructor();

    // After visiting an Op, the result must be written into tmp
    std::unique_ptr<BindingIter> tmp;

    // For path_manager to print in the right direction
    std::vector<bool> begin_at_left;

    // All variables that are assigned in the current scope
    // but the optimizer doesn't know the value (e.g. OPTIONALS)
    std::set<VarId> possible_assigned_vars;

    // The subset of possible assigned vars that always have a non-null value while visiting
    // the current operator and should be fixed in its leaf operators such as IndexScan or Values.
    std::set<VarId> safe_assigned_vars;

private:
    std::vector<VarId> projection_vars;

    std::vector<VarId> group_vars;

    std::vector<VarId> order_vars;
    std::set<VarId>    order_saved_vars;
    std::vector<bool>  order_ascending;

    std::map<VarId, std::unique_ptr<Agg>> aggregations = std::map<VarId, std::unique_ptr<Agg>>();
    std::vector<ProjectionOrderExpr> projection_order_exprs = std::vector<ProjectionOrderExpr>();

    // TODO: the distinct operator can take advantage of order when the DISTINCT variables
    // are a prefix of the ORDER BY variables
    // bool distinct_ordered_possible = false;

    void print_set(const std::set<VarId>& set) const;

    // Checks if the variable is a GROUP BY variable or if it
    // is the result variable for an expressions with aggregation.
    bool is_aggregation_or_group_var(VarId var_id) const;

    // Constructs the operators for GROUP BY, ORDER BY, HAVING, OFFSET, LIMIT, DISTINCT
    // if they are present in the query. Used for SELECT, DESCRIBE, CONSTRUCT and SUB-SELECT.
    void make_solution_modifier_operators(bool is_root_query, bool distinct, uint64_t offset, uint64_t limit);

    // Used to handle common functionality of SELECTs and SUB-SELECTs
    void handle_select(OpSelect& op_select);

    // Calculates the various variable sets needed when making joins.
    JoinVars calculate_join_vars(Op& lhs, Op& rhs);

public:
    void visit(OpOrderBy&)           override;
    void visit(OpGroupBy&)           override;
    void visit(OpHaving&)            override;
    void visit(OpSelect&)            override;
    void visit(OpDescribe&)          override;
    void visit(OpConstruct&)         override;
    void visit(OpWhere&)             override;
    void visit(OpBasicGraphPattern&) override;
    void visit(OpFilter&)            override;
    void visit(OpOptional&)          override;
    void visit(OpUnion&)             override;
    void visit(OpSemiJoin&)          override;
    void visit(OpEmpty&)             override;
    void visit(OpMinus&)             override;
    void visit(OpNotExists&)         override;
    void visit(OpSequence&)          override;
    void visit(OpService&)           override;
    void visit(OpBind&)              override;
    void visit(OpUnitTable&)         override;
    void visit(OpValues&)            override;
    // void visit(OpJoin&)              override;
};
} // namespace SPARQL
