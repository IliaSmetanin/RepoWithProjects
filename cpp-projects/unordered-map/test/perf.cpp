#include "unordered_map.hpp"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include <gtest/gtest.h>

TEST(PerformanceTest, FasterThanStdMap) {
    const int elements = 200000;
    std::vector<int> keys(elements);
    std::iota(keys.begin(), keys.end(), 0);
    std::shuffle(keys.begin(), keys.end(), std::mt19937(42));

    auto start_my = std::chrono::high_resolution_clock::now();
    UnorderedMap<int, int> my_map;
    for (int k : keys) {
        my_map[k] = k;
    }
    for (int k : keys) {
        auto it = my_map.Find(k);
        volatile int sink = it->second;
        (void)sink;
    }
    auto end_my = std::chrono::high_resolution_clock::now();
    auto duration_my =
        std::chrono::duration_cast<std::chrono::milliseconds>(end_my - start_my).count();

    auto start_std = std::chrono::high_resolution_clock::now();
    std::map<int, int> std_map_tree;
    for (int k : keys) {
        std_map_tree[k] = k;
    }
    for (int k : keys) {
        auto it = std_map_tree.find(k);
        volatile int sink = it->second;
        (void)sink;
    }
    auto end_std = std::chrono::high_resolution_clock::now();
    auto duration_std =
        std::chrono::duration_cast<std::chrono::milliseconds>(end_std - start_std).count();

    std::cout << "[ PERFORMANCE ] UnorderedMap: " << duration_my << " ms\n";
    std::cout << "[ PERFORMANCE ] std::map:     " << duration_std << " ms\n";

    EXPECT_LT(duration_my, duration_std);
}

TEST(PerformanceTest, CompareWithStdUnordered) {
    const int elements = 300000;
    std::vector<int> keys(elements);
    std::iota(keys.begin(), keys.end(), 0);
    std::shuffle(keys.begin(), keys.end(), std::mt19937(777));

    auto start_my = std::chrono::high_resolution_clock::now();
    {
        UnorderedMap<int, int> my_map;
        for (int k : keys) {
            my_map[k] = k;
        }
        for (int k : keys) {
            volatile int sink = my_map.At(k);
            (void)sink;
        }
    }
    auto end_my = std::chrono::high_resolution_clock::now();
    auto duration_my =
        std::chrono::duration_cast<std::chrono::milliseconds>(end_my - start_my).count();

    auto start_std = std::chrono::high_resolution_clock::now();
    {
        std::unordered_map<int, int> std_map;
        for (int k : keys) {
            std_map[k] = k;
        }
        for (int k : keys) {
            volatile int sink = std_map.at(k);
            (void)sink;
        }
    }
    auto end_std = std::chrono::high_resolution_clock::now();
    auto duration_std =
        std::chrono::duration_cast<std::chrono::milliseconds>(end_std - start_std).count();

    double diff_percent = (static_cast<double>(duration_my) / duration_std - 1.0) * 100.0;

    std::cout << "\n[ PERFORMANCE REPORT ]\n";
    std::cout << "Your UnorderedMap:    " << duration_my << " ms\n";
    std::cout << "std::unordered_map:   " << duration_std << " ms\n";
    std::cout << "Difference:           " << (diff_percent > 0 ? "+" : "") << diff_percent << "%\n";

    EXPECT_LT(duration_my, duration_std * 5);
}

enum class OpType {
    Insert,
    Erase,
    Contains,
};

struct TwistOp {
    OpType type;
    int key;

    bool operator<(const TwistOp& other) const {
        if (key != other.key) {
            return key < other.key;
        }
        return static_cast<int>(type) < static_cast<int>(other.type);
    }
};

struct SmartParamHasher {
    enum class Mode { AllColliding, AllDifferent, Mixed } mode;

    std::size_t operator()(int key) const noexcept {
        switch (mode) {
            case Mode::AllColliding:
                return 0;
            case Mode::AllDifferent:
                return static_cast<std::size_t>(key);
            case Mode::Mixed:
                return static_cast<std::size_t>(key) % 2;
        }
        return 0;
    }
};

struct TwistParams {
    std::string name;
    SmartParamHasher::Mode mode;
    std::vector<int> keys;
};

class TwistParamTest : public ::testing::TestWithParam<TwistParams> {};

TEST_P(TwistParamTest, StrategyTest) {
    const auto& params = GetParam();

    std::vector<TwistOp> ops;
    for (int k : params.keys) {
        ops.push_back({OpType::Insert, k});
        ops.push_back({OpType::Contains, k});
        ops.push_back({OpType::Erase, k});
    }

    std::sort(ops.begin(), ops.end());

    do {
        SmartParamHasher hasher{params.mode};

        UnorderedMap<int, int, SmartParamHasher> my_map(10, hasher);
        std::unordered_map<int, int, SmartParamHasher> std_map(10, hasher);

        for (const auto& op : ops) {
            if (op.type == OpType::Insert) {
                my_map.Insert({op.key, op.key * 10});
                std_map.insert({op.key, op.key * 10});
            } else if (op.type == OpType::Erase) {
                my_map.Erase(my_map.Find(op.key));
                std_map.erase(op.key);
            } else {
                ASSERT_EQ(my_map.Contains(op.key), std_map.count(op.key) != 0);
            }

            ASSERT_EQ(my_map.Size(), std_map.size());

            for (const auto& [k, v] : std_map) {
                auto it = my_map.Find(k);
                ASSERT_NE(it, my_map.end());
                ASSERT_EQ(it->second, v);
            }
        }
    } while (std::next_permutation(ops.begin(), ops.end()));
}

INSTANTIATE_TEST_SUITE_P(
    TwistScenarios, TwistParamTest,
    ::testing::Values(
        TwistParams{"HardCollisions", SmartParamHasher::Mode::AllColliding, {1, 2, 3}},
        TwistParams{"NoCollisions", SmartParamHasher::Mode::AllDifferent, {1, 2, 3}},
        TwistParams{"MixedBag", SmartParamHasher::Mode::Mixed, {1, 2, 3}}),
    [](const ::testing::TestParamInfo<TwistParams>& info) { return info.param.name; });
