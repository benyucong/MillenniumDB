#include "bplus_tree.h"

#include <cassert>

#include "query/exceptions.h"
#include "storage/buffer_manager.h"
#include "storage/file_manager.h"
#include "storage/index/ordered_file/ordered_file.h"
#include "storage/index/record.h"

template <std::size_t N>
BPlusTree<N>::BPlusTree(const std::string& name) :
    dir_file_id  (file_manager.get_file_id(name + ".dir")),
    leaf_file_id (file_manager.get_file_id(name + ".leaf")),
    root         (BPlusTreeDir<N>(leaf_file_id, buffer_manager.get_page(dir_file_id, 0))) { }


template <std::size_t N>
std::unique_ptr<BPlusTreeDir<N>> BPlusTree<N>::get_root() const noexcept {
    return std::make_unique<BPlusTreeDir<N>>(
        leaf_file_id,
        buffer_manager.get_page(dir_file_id, 0)
    );
}


// template <std::size_t N>
// void BPlusTree<N>::bulk_import(OrderedFile<N>& leaf_provider) {
//     leaf_provider.begin_read();
//     const auto last_page_number = leaf_provider.get_last_page();

//     BPlusTreeLeaf<N> first_leaf(buffer_manager.get_page(leaf_file_id, 0));
//     leaf_provider.copy_to_bpt_leaf(first_leaf, 0);
//     if (last_page_number > 0) {
//         *first_leaf.next_leaf = 1;
//     }
//     first_leaf.page.make_dirty();

//     uint_fast32_t current_page = 1;
//     while (current_page <= last_page_number) {
//         BPlusTreeLeaf<N> new_leaf(buffer_manager.get_page(leaf_file_id, current_page));
//         leaf_provider.copy_to_bpt_leaf(new_leaf, current_page);

//         assert(*new_leaf.value_count > 0);

//         auto next_page = current_page + 1;
//         if (next_page <= last_page_number) {
//             *new_leaf.next_leaf = next_page;
//         }

//         root.bulk_insert(new_leaf);
//         new_leaf.page.make_dirty();
//         current_page = next_page;
//     }
// }


template <std::size_t N>
BptIter<N> BPlusTree<N>::get_range(bool* interruption_requested,
                                   const Record<N>& min,
                                   const Record<N>& max) const noexcept {
    auto leaf_and_pos = root.search_leaf(min);
    return BptIter<N>(interruption_requested, std::move(leaf_and_pos), max);
}


template <std::size_t N>
void BPlusTree<N>::insert(const Record<N>& record) {
    root.insert(record);
}


template <std::size_t N>
bool BPlusTree<N>::check() const {
    return root.check();
}


/******************************* BptIter ********************************/
template <std::size_t N>
const Record<N>* BptIter<N>::next() {
    while (true) {
        if (__builtin_expect(!!(*interruption_requested), 0)) {
            throw InterruptedException();
        }
        if (current_pos < current_leaf.get_value_count()) {
            current_leaf.get_record(current_pos, &current_record);
            // check if res is less than max
            for (unsigned int i = 0; i < N; ++i) {
                if (current_record[i] < max[i]) {
                    ++current_pos;
                    return &current_record;
                }
                else if (current_record[i] > max[i]) {
                    return nullptr;
                }
                // continue iterating if current_record[i] == max[i]
            }
            ++current_pos;
            return &current_record; // res == max
        }
        else if (current_leaf.has_next()) {
            current_leaf.update_to_next_leaf();
            current_pos = 0;
            // continue while
        }
        else {
            return nullptr;
        }
    }
}


template class BPlusTree<1>;
template class BPlusTree<2>;
template class BPlusTree<3>;
template class BPlusTree<4>;

template class BptIter<1>;
template class BptIter<2>;
template class BptIter<3>;
template class BptIter<4>;
