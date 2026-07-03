#include "list.hpp"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <random>
#include <ranges>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include <gtest/gtest.h>

constexpr size_t operator""_sz(unsigned long long int x) {
    return static_cast<size_t>(x);
}

template <typename T, bool PropagateOnCopy, bool PropagateOnMove, bool PropagateOnSwap,
          bool AlwaysEqual = true>
struct PropagatableCountingAllocator : std::allocator<T> {
    using Base = std::allocator<T>;

    using propagate_on_container_copy_assignment = std::bool_constant<PropagateOnCopy>;
    using propagate_on_container_move_assignment = std::bool_constant<PropagateOnMove>;
    using propagate_on_container_swap = std::bool_constant<PropagateOnSwap>;
    using is_always_equal = std::bool_constant<AlwaysEqual>;

    template <typename U>
    struct rebind {
        using other = PropagatableCountingAllocator<U, PropagateOnCopy, PropagateOnMove,
                                                    PropagateOnSwap, AlwaysEqual>;
    };

    PropagatableCountingAllocator() = default;

    explicit PropagatableCountingAllocator(int value) : Base(), value_(value) {
    }

    PropagatableCountingAllocator(const PropagatableCountingAllocator& other)
        : Base(other),
          value_(other.value_),
          allocations_(other.allocations_),
          deallocations_(other.deallocations_) {
    }

    template <typename U, bool P1, bool P2, bool P3, bool P4>
    PropagatableCountingAllocator(const PropagatableCountingAllocator<U, P1, P2, P3, P4>& other)
        : Base(other),
          value_(other.value_),
          allocations_(other.allocations_),
          deallocations_(other.deallocations_) {
    }

    PropagatableCountingAllocator& operator=(const PropagatableCountingAllocator& other) {
        Base::operator=(other);
        value_ = other.value_;
        allocations_ = other.allocations_;
        deallocations_ = other.deallocations_;
        return *this;
    }

    bool operator==(const PropagatableCountingAllocator& other) const {
        return AlwaysEqual || value_ == other.value_;
    }

    bool operator!=(const PropagatableCountingAllocator& other) const {
        return !(*this == other);
    }

    T* allocate(size_t n) {
        ++allocations_;
        return Base::allocate(n);
    }

    void deallocate(T* pointer, size_t n) {
        ++deallocations_;
        Base::deallocate(pointer, n);
    }

    int value_ = 0;
    int allocations_ = 0;
    int deallocations_ = 0;
};

template <class ListType>
void AssertListEqual(const ListType& list, std::initializer_list<int> expected) {
    ASSERT_EQ(list.Size(), expected.size());
    auto it = list.begin();
    auto jt = expected.begin();
    for (; jt != expected.end(); ++jt, ++it) {
        ASSERT_NE(it, list.end());
        ASSERT_EQ(*it, *jt);
    }
    ASSERT_EQ(it, list.end());
}

TEST(List, CopyAssignmentAlloc) {
    {
        using Alloc = PropagatableCountingAllocator<int, true, false, false, true>;

        List<int, Alloc> list1(Alloc{1});
        list1.PushBack(10);
        ASSERT_GT(list1.GetAllocator().allocations_, 0_sz);

        List<int, Alloc> list2(Alloc{2});
        list2 = list1;

        AssertListEqual(list2, {10});
        ASSERT_EQ(list2.GetAllocator().value_, 1);
        ASSERT_GT(list2.GetAllocator().allocations_, 0_sz);
    }

    {
        using Alloc = PropagatableCountingAllocator<int, false, false, false, true>;

        List<int, Alloc> list1(Alloc{1});
        list1.PushBack(10);
        ASSERT_GT(list1.GetAllocator().allocations_, 0_sz);

        List<int, Alloc> list2(Alloc{2});
        list2 = list1;

        AssertListEqual(list2, {10});
        ASSERT_EQ(list2.GetAllocator().value_, 2);
        // ASSERT_GT(list2.GetAllocator().allocations_, 0_sz);
    }

    {
        using Alloc = PropagatableCountingAllocator<int, false, false, false, false>;

        List<int, Alloc> list1(Alloc{1});
        list1.PushBack(10);
        ASSERT_GT(list1.GetAllocator().allocations_, 0_sz);

        List<int, Alloc> list2(Alloc{2});
        list2 = list1;

        AssertListEqual(list2, {10});
        ASSERT_EQ(list2.GetAllocator().value_, 2);
        // ASSERT_GT(list2.GetAllocator().allocations_, 0_sz);
    }
}

TEST(List, MoveAssignmentAlloc) {
    {
        using Alloc = PropagatableCountingAllocator<int, false, true, false, false>;

        List<int, Alloc> src(Alloc{1});
        src.PushBack(1);
        src.PushBack(2);

        List<int, Alloc> dst(Alloc{2});
        dst.PushBack(100);

        dst = std::move(src);

        AssertListEqual(dst, {1, 2});
        ASSERT_EQ(dst.GetAllocator().value_, 1);
    }

    {
        using Alloc = PropagatableCountingAllocator<int, false, false, false, false>;

        List<int, Alloc> src(Alloc{1});
        src.PushBack(1);
        src.PushBack(2);

        List<int, Alloc> dst(Alloc{2});
        dst.PushBack(100);

        dst = std::move(src);

        AssertListEqual(dst, {1, 2});
        ASSERT_EQ(dst.GetAllocator().value_, 2);
    }

    {
        using Alloc = PropagatableCountingAllocator<int, false, false, false, true>;

        List<int, Alloc> src(Alloc{1});
        src.PushBack(1);

        List<int, Alloc> dst(Alloc{2});
        dst = std::move(src);

        AssertListEqual(dst, {1});
        ASSERT_EQ(dst.GetAllocator().value_, 2);
    }
}

TEST(List, SwapAlloc) {
    {
        using Alloc = PropagatableCountingAllocator<int, true, false, true, true>;

        List<int, Alloc> list1(Alloc{1});
        list1.PushBack(1);

        List<int, Alloc> list2(Alloc{2});
        list2.PushBack(2);

        list1.Swap(list2);

        AssertListEqual(list1, {2});
        AssertListEqual(list2, {1});

        ASSERT_EQ(list1.GetAllocator().value_, 2);
        ASSERT_EQ(list2.GetAllocator().value_, 1);
    }

    {
        using Alloc = PropagatableCountingAllocator<int, true, false, false, true>;

        List<int, Alloc> list1(Alloc{1});
        list1.PushBack(1);

        List<int, Alloc> list2(Alloc{2});
        list2.PushBack(2);

        list1.Swap(list2);

        AssertListEqual(list1, {2});
        AssertListEqual(list2, {1});

        ASSERT_EQ(list1.GetAllocator().value_, 1);
        ASSERT_EQ(list2.GetAllocator().value_, 2);
    }
}

TEST(List, SwapDoesNotInvalidateIterators) {
    using Alloc = PropagatableCountingAllocator<int, true, false, true, true>;

    List<int, Alloc> list1(Alloc{1});
    List<int, Alloc> list2(Alloc{2});

    list1.PushBack(10);
    list2.PushBack(20);

    auto it1 = list1.begin();
    auto it2 = list2.begin();

    int* ptr1 = &*it1;
    int* ptr2 = &*it2;

    list1.Swap(list2);

    ASSERT_EQ(*ptr1, 10);
    ASSERT_EQ(*ptr2, 20);
    ASSERT_EQ(*it1, 10);
    ASSERT_EQ(*it2, 20);

    AssertListEqual(list1, {20});
    AssertListEqual(list2, {10});
}