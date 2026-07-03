#include "list.hpp"

#include <iterator>
#include <list>
#include <random>
#include <string>

#include <gtest/gtest.h>

TEST(Fuzzing, FullScaleFuzzing) {
    enum Operation {
        push_back,
        push_front,
        pop_back,
        pop_front,
        insert_at_iterator,
        erase_at_iterator,
        front_and_back_access,
        size_and_empty,
        r_iter_sum,
        clear,
        copy,
        assignment
    };

    uint32_t seed = 67;

    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> value_dist(1, 10000);
    std::uniform_int_distribution<int> op_dist(0, 11);

    List<int> my_list;
    std::list<int> std_list;

    constexpr int kNumOperations = 50000;

    for (int i = 0; i < kNumOperations; ++i) {
        Operation op = Operation(op_dist(rng));
        switch (op) {
            case push_back: {
                int val = value_dist(rng);
                my_list.PushBack(val);
                std_list.push_back(val);
                break;
            }
            case push_front: {
                int val = value_dist(rng);
                my_list.PushFront(val);
                std_list.push_front(val);
                break;
            }
            case pop_back: {
                if (!std_list.empty()) {
                    my_list.PopBack();
                    std_list.pop_back();
                }
                break;
            }
            case pop_front: {
                if (!std_list.empty()) {
                    my_list.PopFront();
                    std_list.pop_front();
                }
                break;
            }
            case insert_at_iterator: {
                int pos = std_list.empty() ? 0 : rng() % (std_list.size() + 1);
                auto my_it = my_list.begin();
                auto std_it = std_list.begin();
                std::advance(my_it, pos);
                std::advance(std_it, pos);
                int val = value_dist(rng);
                my_list.Insert(my_it, val);
                std_list.insert(std_it, val);
                break;
            }
            case erase_at_iterator: {
                if (!std_list.empty()) {
                    int pos = rng() % std_list.size();
                    auto my_it = my_list.begin();
                    auto std_it = std_list.begin();
                    std::advance(my_it, pos);
                    std::advance(std_it, pos);
                    std_list.erase(std_it);
                    my_list.Erase(my_it);
                }
                break;
            }
            case front_and_back_access: {
                if (!std_list.empty()) {
                    ASSERT_EQ(*my_list.begin(), std_list.front());
                    ASSERT_EQ(*my_list.rbegin(), std_list.back());
                }
                break;
            }
            case size_and_empty: {
                ASSERT_EQ(my_list.Size(), std_list.size());
                ASSERT_EQ(my_list.Empty(), std_list.empty());
                break;
            }
            case r_iter_sum: {
                int64_t my_sum = 0;
                for (auto it = my_list.crbegin(); it != my_list.crend(); ++it) {
                    my_sum += *it;
                }
                int64_t std_sum = 0;
                for (auto it = std_list.rbegin(); it != std_list.rend(); ++it) {
                    std_sum += *it;
                }
                ASSERT_EQ(my_sum, std_sum);
                break;
            }
            case clear: {
                if (i % 1000 == 0) {
                    my_list.Clear();
                    std_list.clear();
                }
                break;
            }
            case copy: {
                if (i % 2000 == 0) {
                    List<int> copy(my_list);
                    ASSERT_TRUE(copy == my_list);
                }
                break;
            }
            case assignment: {
                if (i % 2000 == 0) {
                    List<int> assigned;
                    assigned = my_list;
                    ASSERT_TRUE(assigned == my_list);
                }
                break;
            }
        }

        if (i % 500 == 0) {
            ASSERT_EQ(my_list.Size(), std_list.size())
                << "Mismatch at iteration " << i << "or up to 500 iters before";
            auto my_it = my_list.begin();
            auto std_it = std_list.begin();
            while (std_it != std_list.end()) {
                ASSERT_EQ(*my_it, *std_it);
                ++my_it;
                ++std_it;
            }
        }
    }
}
