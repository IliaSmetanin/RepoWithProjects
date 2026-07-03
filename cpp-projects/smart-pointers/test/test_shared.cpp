#include "smart_pointers.hpp"

#include <algorithm>
#include <memory>
#include <type_traits>
#include <vector>

#include <gtest/gtest.h>

struct Base {
    virtual ~Base() {
    }
};

struct Derived : public Base {};

struct VoidTestObject {
    inline static int created = 0;
    inline static int destroyed = 0;

    VoidTestObject() {
        ++created;
    }

    ~VoidTestObject() {
        ++destroyed;
    }
};

struct VerySpecialTypeForAntiStd {};

template <int Tag>
struct TaggedDestructionCounter {
    inline static int destroyed = 0;

    static int GetTag() {
        return Tag;
    }

    ~TaggedDestructionCounter() {
        ++destroyed;
    }
};

std::pair<SharedPtr<std::vector<int>>, SharedPtr<std::vector<int>>> MakePtrs() {
    std::pair<SharedPtr<std::vector<int>>, SharedPtr<std::vector<int>>> ptrs;

    ptrs.first = SharedPtr<std::vector<int>>(new std::vector<int>(1'000'000));
    (*ptrs.first)[0] = 1;

    std::vector<int>& vec = *ptrs.first;
    ptrs.second = SharedPtr<std::vector<int>>(new std::vector<int>(vec));
    (*ptrs.second)[0] = 2;

    return ptrs;
}

TEST(SharedPtr, Swap) {
    auto [first_ptr, second_ptr] = MakePtrs();

    for (int i = 0; i < 1'000'000; ++i) {
        first_ptr.Swap(second_ptr);
    }
    first_ptr->swap(*second_ptr);

    ASSERT_EQ(first_ptr->front(), 2);
    ASSERT_EQ(second_ptr->front(), 1);

    ASSERT_EQ(first_ptr.UseCount(), 1);
    ASSERT_EQ(second_ptr.UseCount(), 1);

    auto& vec = *first_ptr;

    for (int i = 0; i < 10; ++i) {
        auto third_ptr = SharedPtr<std::vector<int>>(new std::vector<int>(vec));
        auto fourth_ptr = second_ptr;
        fourth_ptr.Swap(third_ptr);
        ASSERT_EQ(second_ptr.UseCount(), 2);
    }

    ASSERT_EQ(second_ptr.UseCount(), 1);
}

TEST(SharedPtr, MultipleUsers) {
    auto [first_ptr, second_ptr] = MakePtrs();

    std::vector<SharedPtr<std::vector<int>>> ptrs(10, SharedPtr<std::vector<int>>(first_ptr));
    for (int i = 0; i < 100'000; ++i) {
        ptrs.push_back(ptrs.back());
        ptrs.push_back(SharedPtr<std::vector<int>>(ptrs.back()));
    }
    ASSERT_EQ(first_ptr.UseCount(), 1 + 10 + 200'000);
}

TEST(SharedPtr, Reset) {
    auto [first_ptr, second_ptr] = MakePtrs();

    first_ptr.Reset(new std::vector<int>());
    second_ptr.Reset();
    SharedPtr<std::vector<int>>().Swap(first_ptr);

    ASSERT_EQ(second_ptr.Get(), nullptr);
    ASSERT_EQ(second_ptr.Get(), nullptr);
}

TEST(SharedPtr, Workflow) {
    for (int k = 0; k < 2; ++k) {
        std::vector<SharedPtr<int>> ptrs;
        for (int i = 0; i < 100'000; ++i) {
            int* p = new int(rand() % 99'999);
            ptrs.push_back(SharedPtr<int>(p));
        }
        std::sort(ptrs.begin(), ptrs.end(), [](auto&& x, auto&& y) { return *x < *y; });
        for (int i = 0; i + 1 < 100'000; ++i) {
            ASSERT_LE(*(ptrs[i]), *(ptrs[i + 1]));
        }
        while (!ptrs.empty()) {
            ptrs.pop_back();
        }
    }
}

TEST(SharedPtr, Constness) {
    const SharedPtr<int> sp(new int(42));
    ASSERT_EQ(sp.UseCount(), 1);
    ASSERT_EQ(*sp.Get(), 42);
    ASSERT_EQ(*sp, 42);
}

TEST(SharedPtr, AliasingConstructor) {
    SharedPtr<TaggedDestructionCounter<0>> initial(new TaggedDestructionCounter<0>{});
    TaggedDestructionCounter<1>* unmanaged = new TaggedDestructionCounter<1>{};
    SharedPtr<TaggedDestructionCounter<1>> counter_holder(initial, unmanaged);

    ASSERT_EQ(initial.UseCount(), 2);
    ASSERT_EQ(initial->GetTag(), 0);
    ASSERT_EQ(counter_holder.UseCount(), 2);
    ASSERT_EQ(counter_holder->GetTag(), 1);

    initial.Reset();
    ASSERT_EQ(TaggedDestructionCounter<0>::destroyed, 0);
    ASSERT_EQ(TaggedDestructionCounter<1>::destroyed, 0);
    ASSERT_EQ(counter_holder.UseCount(), 1);

    counter_holder.Reset();
    ASSERT_EQ(TaggedDestructionCounter<0>::destroyed, 1);
    ASSERT_EQ(TaggedDestructionCounter<1>::destroyed, 0);

    delete unmanaged;
}

TEST(SharedPtr, SfinaeConvertibility) {
    static_assert(std::is_constructible_v<SharedPtr<const int>, const SharedPtr<int>&>);
    static_assert(std::is_constructible_v<SharedPtr<const int>, SharedPtr<int>&&>);
    static_assert(std::is_assignable_v<SharedPtr<const int>&, const SharedPtr<int>&>);
    static_assert(std::is_assignable_v<SharedPtr<const int>&, SharedPtr<int>&&>);

    static_assert(!std::is_constructible_v<SharedPtr<int>, const SharedPtr<const int>&>);
    static_assert(!std::is_constructible_v<SharedPtr<int>, SharedPtr<const int>&&>);
    static_assert(!std::is_assignable_v<SharedPtr<int>&, const SharedPtr<const int>&>);
    static_assert(!std::is_assignable_v<SharedPtr<int>&, SharedPtr<const int>&&>);

    static_assert(std::is_constructible_v<SharedPtr<Base>, const SharedPtr<Derived>&>);
    static_assert(std::is_constructible_v<SharedPtr<Base>, SharedPtr<Derived>&&>);
    static_assert(std::is_assignable_v<SharedPtr<Base>&, const SharedPtr<Derived>&>);
    static_assert(std::is_assignable_v<SharedPtr<Base>&, SharedPtr<Derived>&&>);

    static_assert(!std::is_constructible_v<SharedPtr<Base>, const SharedPtr<const Derived>&>);
    static_assert(!std::is_assignable_v<SharedPtr<Base>&, const SharedPtr<const Derived>&>);

    static_assert(std::is_constructible_v<SharedPtr<const Base>, const SharedPtr<Derived>&>);
    static_assert(std::is_constructible_v<SharedPtr<const Base>, const SharedPtr<const Derived>&>);
}

TEST(SharedPtr, AntiStdDerivationCheck) {
    static_assert(!std::is_base_of_v<std::shared_ptr<VerySpecialTypeForAntiStd>,
                                     SharedPtr<VerySpecialTypeForAntiStd>>,
                  "Don't try to use std smart pointers");

    static_assert(!std::is_base_of_v<std::weak_ptr<VerySpecialTypeForAntiStd>,
                                     WeakPtr<VerySpecialTypeForAntiStd>>,
                  "Don't try to use std smart pointers");
}
