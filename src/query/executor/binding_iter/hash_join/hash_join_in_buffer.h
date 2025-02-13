#pragma once

#include <memory>
#include <vector>

#include "query/executor/binding_iter.h"
#include "storage/index/hash/key_value_hash/key_value_hash.h"
#include "storage/index/hash/key_value_hash/key_value_pair_hasher.h"

class HashJoinInBuffer : public BindingIter {
public:
    HashJoinInBuffer(
        std::unique_ptr<BindingIter> lhs,
        std::unique_ptr<BindingIter> rhs,
        std::vector<VarId>             left_vars,
        std::vector<VarId>             common_vars,
        std::vector<VarId>             right_vars
    ) :
        lhs           (std::move(lhs)),
        rhs           (std::move(rhs)),
        left_vars     (left_vars),
        common_vars   (common_vars),
        right_vars    (right_vars),
        lhs_hash      (KeyValueHash<ObjectId, ObjectId>(common_vars.size(), left_vars.size())) { }

    void analyze(std::ostream& os, int indent = 0) const override;
    void begin(Binding& parent_binding) override;
    bool next() override;
    void reset() override;
    void assign_nulls() override;

private:
    std::unique_ptr<BindingIter> lhs;
    std::unique_ptr<BindingIter> rhs;
    std::vector<VarId> left_vars;
    std::vector<VarId> common_vars;
    std::vector<VarId> right_vars;

    Binding* parent_binding;

    // true if we found a value from rhs (current_key and current_value) and we got a pre-match (same hash to lhs bucket).
    // In this state we have to iterate over all the keys in that bucket (`current_bucket`) to check if
    // they have the same key. Once the iterating is over, enumerating turns into false.
    bool enumerating;

    KeyValueHash<ObjectId, ObjectId> lhs_hash;  // assuming lhs is the smaller one (from execution plan)
    uint_fast32_t current_bucket;
    uint_fast32_t current_bucket_pos;

    std::vector<ObjectId> current_key;
    std::vector<ObjectId> current_value;
};
