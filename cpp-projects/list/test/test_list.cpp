#include "list.hpp"
#include "pool_allocator.hpp"

#include "test/common.hpp"
#include "test/stack_allocation.hpp"

#include <algorithm>
#include <cstddef>
#include <deque>
#include <exception>
#include <iterator>
#include <list>
#include <memory>
#include <type_traits>
#include <vector>

#include <gtest/gtest.h>
#include <sys/resource.h>

// template <typename T, typename Alloc = std::allocator<T>>
// using List = std::list<T, Alloc>;

// template <typename T, size_t>
// using PoolAllocator = std::allocator<T>;

struct CustomAllocatorException {
    const char* const message;

    CustomAllocatorException(const char* const message) : message(message) {
    }

    const char* What() const noexcept {
        return message;
    }
};

template <typename T>
struct ExceptionalAllocator {
    using value_type = T;

    size_t time_to_exception;
    std::allocator<T> actual_allocator;

    ExceptionalAllocator(size_t time) : time_to_exception(time) {
    }

    template <typename U>
    ExceptionalAllocator(const ExceptionalAllocator<U>& other)
        : time_to_exception(other.time_to_exception) {
    }

    T* allocate(size_t n) {
        if (time_to_exception == 0) {
            throw CustomAllocatorException("this is exceptional");
        }
        if (time_to_exception < n) {
            time_to_exception = 0;
        } else {
            time_to_exception -= n;
        }
        return actual_allocator.allocate(n);
    }

    void deallocate(T* pointer, size_t n) {
        return actual_allocator.deallocate(pointer, n);
    }

    bool operator==(const ExceptionalAllocator& other) const = default;
};

struct FragileException : public std::exception {};

struct Fragile {
    Fragile(int durability, int data) : durability(durability), data(data) {
    }

    ~Fragile() = default;

    // for std::swap
    Fragile(Fragile&& other) : Fragile() {
        *this = other;
    }

    Fragile(const Fragile& other) : Fragile() {
        *this = other;
    }

    Fragile& operator=(const Fragile& other) {
        durability = other.durability - 1;
        data = other.data;
        if (durability <= 0) {
            throw FragileException{};
        }
        return *this;
    }

    Fragile& operator=(Fragile&& other) {
        return *this = other;
    }

    int durability = -1;
    int data = -1;

private:
    Fragile() {
    }
};

struct ExplosiveException : public std::exception {};

struct Explosive {
    struct Safeguard {};

    inline static bool exploded = false;

    Explosive() : should_explode_(true) {
        throw ExplosiveException{};
    }

    Explosive(Safeguard) : should_explode_(false) {
    }

    Explosive(const Explosive&) : should_explode_(true) {
        throw ExplosiveException{};
    }

    Explosive& operator=(const Explosive&) {
        return *this;
    }

    ~Explosive() {
        exploded |= should_explode_;
    }

private:
    const bool should_explode_;
};

struct DefaultConstructible {
    DefaultConstructible() : data(kDefaultData) {
    }

    int data = kDefaultData;
    inline static const int kDefaultData = 117;
};

struct NotDefaultConstructible {
    NotDefaultConstructible() = delete;

    NotDefaultConstructible(int input) : data(input) {
    }

    int data;

    auto operator<=>(const NotDefaultConstructible&) const = default;
};

template <typename Iter, typename T>
struct CheckIter {
    using Traits = std::iterator_traits<Iter>;

    static_assert(
        std::is_same_v<std::remove_cv_t<typename Traits::value_type>, std::remove_cv_t<T>>);
    static_assert(std::is_same_v<typename Traits::pointer, T*>);
    static_assert(std::is_same_v<typename Traits::reference, T&>);
    static_assert(
        std::is_same_v<typename Traits::iterator_category, std::bidirectional_iterator_tag>);

    static_assert(std::is_same_v<decltype(std::declval<Iter>()++), Iter>);
    static_assert(std::is_same_v<decltype(++std::declval<Iter>()), Iter&>);

    static_assert(std::is_same_v<decltype(std::declval<Iter>()--), Iter>);
    static_assert(std::is_same_v<decltype(--std::declval<Iter>()), Iter&>);

    static_assert(std::is_same_v<decltype(*std::declval<Iter>()), T&>);

    static_assert(std::is_same_v<decltype(std::declval<Iter>() == std::declval<Iter>()), bool>);
    static_assert(std::is_same_v<decltype(std::declval<Iter>() != std::declval<Iter>()), bool>);
};

template <typename T>
void AssignToSelf(T& value) {
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
    value = value;
#ifdef __clang__
#pragma clang diagnostic pop
#endif
}

template <typename T, typename Alloc>
bool CheckEqual(std::initializer_list<T> expected, List<T, Alloc>& actual) {
    if (expected.size() != actual.Size()) {
        return false;
    }

    std::vector<T> container{expected};
    return std::equal(container.begin(), container.end(), actual.begin());
}

TEST(List, Sanity) {
    static_assert(!(std::is_assignable_v<List<int>, std::list<int>> ||
                    std::is_assignable_v<std::list<int>, List<int>>));
}

TEST(List, TestNoAllocationsOnEmptyList) {
    using AllocCounter = AllocatorCounterWrap<int>;

    AllocCounter alloc_counter;

    List<int, AllocCounter> lst(alloc_counter);

    assert(AllocCounterContainer::alloc_calls == 0 &&
           "List shouldn't allocate anything in default constructor (empty list)");
    assert(AllocCounterContainer::dealloc_calls == 0 && "and deallocate too");
}

TEST(ListComparison, EmptyAndSingleElementCases) {
    {
        List<int> lhs;
        List<int> rhs;

        ASSERT_TRUE(lhs == rhs);
        ASSERT_FALSE(lhs != rhs);

        rhs.PushBack(1);

        ASSERT_FALSE(lhs == rhs);
        ASSERT_TRUE(lhs != rhs);
    }

    {
        List<int> lhs;
        List<int> rhs;
        lhs.PushBack(42);
        rhs.PushBack(42);

        ASSERT_TRUE(lhs == rhs);
        ASSERT_FALSE(lhs != rhs);
    }

    {
        List<int> lhs;
        List<int> rhs;
        lhs.PushBack(42);
        rhs.PushBack(43);

        ASSERT_FALSE(lhs == rhs);
        ASSERT_TRUE(lhs != rhs);
    }

    {
        List<int> list;
        list.PushBack(7);

        ASSERT_TRUE(list == list);
        ASSERT_FALSE(list != list);
    }
}

TEST(ListComparison, SameSizeAndDifferentSizeCases) {
    {
        List<int> lhs;
        List<int> rhs;

        for (int x : {1, 2, 3, 4, 5}) {
            lhs.PushBack(x);
            rhs.PushBack(x);
        }

        ASSERT_TRUE(lhs == rhs);
        ASSERT_FALSE(lhs != rhs);
    }

    {
        List<int> lhs;
        List<int> rhs;

        for (int x : {1, 2, 3, 4, 5}) {
            lhs.PushBack(x);
        }
        for (int x : {1, 2, 3, 4}) {
            rhs.PushBack(x);
        }

        ASSERT_FALSE(lhs == rhs);
        ASSERT_TRUE(lhs != rhs);
    }

    {
        List<int> lhs;
        List<int> rhs;

        for (int x : {1, 2, 3, 4}) {
            lhs.PushBack(x);
            rhs.PushBack(x);
        }
        rhs.PushBack(5);

        ASSERT_FALSE(lhs == rhs);
        ASSERT_TRUE(lhs != rhs);
    }
}

TEST(ListComparison, DifferencePositionMatters) {
    {
        List<int> lhs;
        List<int> rhs;

        for (int x : {10, 2, 3, 4, 5}) {
            lhs.PushBack(x);
        }
        for (int x : {11, 2, 3, 4, 5}) {
            rhs.PushBack(x);
        }

        ASSERT_FALSE(lhs == rhs);
        ASSERT_TRUE(lhs != rhs);
    }

    {
        List<int> lhs;
        List<int> rhs;

        for (int x : {1, 2, 30, 4, 5}) {
            lhs.PushBack(x);
        }
        for (int x : {1, 2, 31, 4, 5}) {
            rhs.PushBack(x);
        }

        ASSERT_FALSE(lhs == rhs);
        ASSERT_TRUE(lhs != rhs);
    }

    {
        List<int> lhs;
        List<int> rhs;

        for (int x : {1, 2, 3, 4, 50}) {
            lhs.PushBack(x);
        }
        for (int x : {1, 2, 3, 4, 51}) {
            rhs.PushBack(x);
        }

        ASSERT_FALSE(lhs == rhs);
        ASSERT_TRUE(lhs != rhs);
    }

    {
        List<int> lhs;
        List<int> rhs;

        for (int x : {1, 2, 3, 4, 5}) {
            lhs.PushBack(x);
        }
        for (int x : {5, 4, 3, 2, 1}) {
            rhs.PushBack(x);
        }

        ASSERT_FALSE(lhs == rhs);
        ASSERT_TRUE(lhs != rhs);
    }
}

TEST(ListComparison, EqualityAfterDifferentConstructionPaths) {
    {
        List<int> lhs;
        lhs.PushBack(1);
        lhs.PushBack(2);
        lhs.PushBack(3);

        List<int> rhs;
        rhs.PushFront(3);
        rhs.PushFront(2);
        rhs.PushFront(1);

        ASSERT_TRUE(lhs == rhs);
        ASSERT_FALSE(lhs != rhs);
    }

    {
        List<int> lhs;
        List<int> rhs;
        rhs.PushBack(0);
        for (int x : {1, 2, 3, 4, 5}) {
            lhs.PushBack(x);
            rhs.PushBack(x);
        }
        rhs.PopFront();

        ASSERT_TRUE(lhs == rhs);
        ASSERT_FALSE(lhs != rhs);
    }

    {
        List<int> lhs;
        for (int x : {1, 2, 3, 4, 5}) {
            lhs.PushBack(x);
        }

        List<int> rhs;
        for (int x : {1, 2, 100, 3, 4, 5}) {
            rhs.PushBack(x);
        }
        auto it = rhs.begin();
        std::advance(it, 2);
        rhs.Erase(it);

        ASSERT_TRUE(lhs == rhs);
        ASSERT_FALSE(lhs != rhs);
    }
}

TEST(ListComparison, EqualityChangesAfterMutation) {
    {
        List<int> lhs;
        List<int> rhs;

        for (int x : {1, 2, 3}) {
            lhs.PushBack(x);
            rhs.PushBack(x);
        }

        ASSERT_TRUE(lhs == rhs);
        ASSERT_FALSE(lhs != rhs);

        rhs.PopBack();
        rhs.PushBack(4);

        ASSERT_FALSE(lhs == rhs);
        ASSERT_TRUE(lhs != rhs);

        rhs.PopBack();
        rhs.PushBack(3);

        ASSERT_TRUE(lhs == rhs);
        ASSERT_FALSE(lhs != rhs);
    }

    {
        List<int> lhs;
        List<int> rhs;

        for (int x : {1, 2, 3, 4}) {
            lhs.PushBack(x);
            rhs.PushBack(x);
        }

        auto it = rhs.begin();
        std::advance(it, 2);
        rhs.Erase(it);

        ASSERT_FALSE(lhs == rhs);
        ASSERT_TRUE(lhs != rhs);

        it = rhs.begin();
        std::advance(it, 2);
        rhs.Insert(it, 3);

        ASSERT_TRUE(lhs == rhs);
        ASSERT_FALSE(lhs != rhs);
    }

    {
        List<int> lhs;
        for (int x : {1, 2, 3, 4, 5}) {
            lhs.PushBack(x);
        }

        List<int> rhs = lhs;
        ASSERT_TRUE(lhs == rhs);
        ASSERT_FALSE(lhs != rhs);

        lhs.PopFront();
        ASSERT_FALSE(lhs == rhs);
        ASSERT_TRUE(lhs != rhs);

        rhs.PopFront();
        ASSERT_TRUE(lhs == rhs);
        ASSERT_FALSE(lhs != rhs);
    }
}

TEST(ListComparison, CopyAndSwapDerivedEquality) {
    {
        List<int> lhs;
        for (int x : {1, 2, 3, 4}) {
            lhs.PushBack(x);
        }

        List<int> rhs = lhs;
        ASSERT_TRUE(lhs == rhs);
        ASSERT_FALSE(lhs != rhs);
    }

    {
        List<int> lhs;
        List<int> rhs;

        for (int x : {1, 2, 3}) {
            lhs.PushBack(x);
        }
        for (int x : {4, 5}) {
            rhs.PushBack(x);
        }

        ASSERT_FALSE(lhs == rhs);
        ASSERT_TRUE(lhs != rhs);

        rhs.Swap(lhs);

        ASSERT_FALSE(lhs == rhs);
        ASSERT_TRUE(lhs != rhs);

        List<int> expected;
        for (int x : {1, 2, 3}) {
            expected.PushBack(x);
        }

        ASSERT_TRUE(rhs == expected);
        ASSERT_FALSE(rhs != expected);
    }

    {
        List<int> lhs;
        for (int x : {9, 8, 7}) {
            lhs.PushBack(x);
        }

        List<int> rhs;
        rhs.PushBack(1);
        rhs.PushBack(2);

        rhs = lhs;

        ASSERT_TRUE(lhs == rhs);
        ASSERT_FALSE(lhs != rhs);
    }
}

TEST(ListComparison, NotOperatorConsistency) {
    {
        List<int> lhs;
        List<int> rhs;

        ASSERT_EQ(lhs != rhs, !(lhs == rhs));
        ASSERT_EQ(rhs != lhs, !(rhs == lhs));
    }

    {
        List<int> lhs;
        List<int> rhs;

        for (int x : {1, 2, 3}) {
            lhs.PushBack(x);
            rhs.PushBack(x);
        }

        ASSERT_EQ(lhs != rhs, !(lhs == rhs));
        ASSERT_EQ(rhs != lhs, !(rhs == lhs));
    }

    {
        List<int> lhs;
        List<int> rhs;

        for (int x : {1, 2, 3}) {
            lhs.PushBack(x);
        }
        for (int x : {1, 2, 4}) {
            rhs.PushBack(x);
        }

        ASSERT_EQ(lhs != rhs, !(lhs == rhs));
        ASSERT_EQ(rhs != lhs, !(rhs == lhs));
    }
}

TEST(List, ConstructionAssertions) {
    using T1 = int;
    using T2 = NotDefaultConstructible;

    static_assert(std::is_default_constructible_v<List<T1>>, "should have default constructor");
    static_assert(std::is_default_constructible_v<List<T2>>, "should have default constructor");
    static_assert(std::is_copy_constructible_v<List<T1>>, "should have copy constructor");
    static_assert(std::is_copy_constructible_v<List<T2>>, "should have copy constructor");
    static_assert(std::is_constructible_v<List<T1>, size_t, const T1&>,
                  "should have constructor from size_t and const T&");
    static_assert(std::is_constructible_v<List<T2>, size_t, const T2&>,
                  "should have constructor from size_t and const T&");

    static_assert(std::is_copy_assignable_v<List<T1>>, "should have assignment operator");
    static_assert(std::is_copy_assignable_v<List<T2>>, "should have assignment operator");
}

TEST(List, DefaultConstructor) {
    List<int> defaulted;
    ASSERT_TRUE(defaulted.Empty());

    List<NotDefaultConstructible> without_default;
    ASSERT_TRUE(without_default.Empty());
}

TEST(List, CopyConstructor) {
    List<NotDefaultConstructible> without_default;
    List<NotDefaultConstructible> copy = without_default;
    ASSERT_TRUE(copy.Empty());
}

TEST(List, SizeConstructor) {
    size_t size = kSmallSize;
    int value = kNontrivialInt;

    List<int> simple(size);
    ASSERT_EQ(simple.Size(), size);
    ASSERT_TRUE(std::all_of(simple.begin(), simple.end(), [](int item) { return item == 0; }));

    List<NotDefaultConstructible> less_simple(size, value);
    ASSERT_EQ(less_simple.Size(), size);
    ASSERT_TRUE(std::all_of(less_simple.begin(), less_simple.end(),
                            [&](const auto& item) { return item.data == value; }));

    List<DefaultConstructible> default_constructor(size);
    ASSERT_TRUE(std::all_of(
        default_constructor.begin(), default_constructor.end(),
        [](const auto& item) { return item.data == DefaultConstructible::kDefaultData; }));
}

TEST(List, Assignment) {
    List<int> first(kSmallSize, kNontrivialInt);
    const auto second_size = kSmallSize - 1;
    List<int> second(kSmallSize - 1, kNontrivialInt - 1);

    first = second;
    ASSERT_EQ(first.Size(), second.Size());
    ASSERT_EQ(first.Size(), second_size);
    ASSERT_TRUE(std::equal(first.begin(), first.end(), second.begin()));

    AssignToSelf(second);
    ASSERT_EQ(first.Size(), second.Size());
    ASSERT_EQ(first.Size(), second_size);
    ASSERT_TRUE(std::equal(first.begin(), first.end(), second.begin()));
}

TEST(List, MultipleSelfAssignment) {
    List<int> list(kBigSize, kNontrivialInt);
    List<int> list_copy = list;

    constexpr size_t kIterCount = 10'000'000;
    for (size_t _ = 0; _ < kIterCount; ++_) {
        AssignToSelf(list);
    }
    ASSERT_TRUE(std::equal(list.begin(), list.end(), list_copy.begin()));
}

template <typename T, bool PropagateOnCopy, bool PropagateOnSwap, bool AlwaysEqual = true>
struct PropagatableCountingAllocator : std::allocator<T> {
    using Base = std::allocator<T>;

    using propagate_on_container_copy_assignment = std::bool_constant<PropagateOnCopy>;
    using propagate_on_container_swap = std::bool_constant<PropagateOnSwap>;

    template <typename U>
    struct rebind {
        using other =
            PropagatableCountingAllocator<U, PropagateOnCopy, PropagateOnSwap, AlwaysEqual>;
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

    template <typename U, bool P1, bool P2, bool P3>
    PropagatableCountingAllocator(const PropagatableCountingAllocator<U, P1, P2, P3>& other)
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

TEST(List, EmplaceSingleArgument) {
    {
        List<NotDefaultConstructible> list;

        list.EmplaceBack(17);

        ASSERT_EQ(list.Size(), 1);
        ASSERT_EQ(list.begin()->data, 17);
        ASSERT_EQ((--list.end())->data, 17);
    }
    {
        List<NotDefaultConstructible> list;

        list.EmplaceFront(17);

        ASSERT_EQ(list.Size(), 1);
        ASSERT_EQ(list.begin()->data, 17);
        ASSERT_EQ((--list.end())->data, 17);
    }
}

TEST(List, EmplaceSeveralElementsOrder) {
    {
        List<NotDefaultConstructible> list;

        list.EmplaceBack(1);
        list.EmplaceBack(2);
        list.EmplaceBack(3);

        ASSERT_EQ(list.Size(), 3);

        auto it = list.begin();
        ASSERT_EQ(it->data, 1);
        ++it;
        ASSERT_EQ(it->data, 2);
        ++it;
        ASSERT_EQ(it->data, 3);
    }
    {
        List<NotDefaultConstructible> list;

        list.EmplaceFront(1);
        list.EmplaceFront(2);
        list.EmplaceFront(3);

        ASSERT_EQ(list.Size(), 3);

        auto it = list.begin();
        ASSERT_EQ(it->data, 3);
        ++it;
        ASSERT_EQ(it->data, 2);
        ++it;
        ASSERT_EQ(it->data, 1);
    }
}

TEST(List, EmplaceIntoNonEmptyList) {
    {
        List<NotDefaultConstructible> list;

        list.EmplaceBack(10);
        list.EmplaceFront(5);
        list.EmplaceBack(20);

        ASSERT_EQ(list.Size(), 3);

        auto it = list.begin();
        ASSERT_EQ(it->data, 5);
        ++it;
        ASSERT_EQ(it->data, 10);
        ++it;
        ASSERT_EQ(it->data, 20);
    }
    {
        List<NotDefaultConstructible> list;

        list.EmplaceFront(10);
        list.EmplaceBack(20);
        list.EmplaceFront(5);

        ASSERT_EQ(list.Size(), 3);

        auto it = list.begin();
        ASSERT_EQ(it->data, 5);
        ++it;
        ASSERT_EQ(it->data, 10);
        ++it;
        ASSERT_EQ(it->data, 20);
    }
}

TEST(List, AssignmentAlloc) {
    {
        using Alloc = PropagatableCountingAllocator<int, true, false, true>;
        List<int, Alloc> list1(Alloc{1});
        list1.PushBack(1);
        ASSERT_EQ(list1.GetAllocator().allocations_, 1);

        List<int, Alloc> list2(Alloc{2});
        list2 = list1;
        ASSERT_EQ(list2.GetAllocator().allocations_, 2);
        ASSERT_EQ(list2.GetAllocator().value_, 1);
    }

    {
        using Alloc = PropagatableCountingAllocator<int, false, false, true>;
        List<int, Alloc> list1(Alloc{1});
        list1.PushBack(1);
        ASSERT_EQ(list1.GetAllocator().allocations_, 1);

        List<int, Alloc> list2(Alloc{2});
        list2 = list1;
        ASSERT_EQ(list2.GetAllocator().allocations_, 1);
        ASSERT_EQ(list2.GetAllocator().value_, 2);
    }

    {
        using Alloc = PropagatableCountingAllocator<int, false, false, false>;
        List<int, Alloc> list1(Alloc{1});
        list1.PushBack(1);
        ASSERT_EQ(list1.GetAllocator().allocations_, 1);

        List<int, Alloc> list2(Alloc{2});
        list2 = list1;
        ASSERT_EQ(list2.GetAllocator().allocations_, 1);
        ASSERT_EQ(list2.GetAllocator().value_, 2);
    }
}

TEST(List, PushPop) {
    List<int> list;

    list.PushBack(3);
    list.PushBack(4);
    list.PushFront(2);
    list.PushBack(5);
    list.PushFront(1);

    ASSERT_EQ(list.Size(), 5);
    ASSERT_TRUE(std::equal(list.begin(), list.end(), IotaIterator<int>{1}));

    std::reverse(list.begin(), list.end());

    ASSERT_EQ(list.Size(), 5);
    ASSERT_TRUE(std::equal(list.begin(), list.end(), ReversedIotaIterator<int>{5}));

    list.PopFront();
    list.PopBack();
    list.PopBack();

    ASSERT_TRUE(CheckEqual({4, 3}, list));
}

TEST(List, InsertErase) {
    List<int> list;

    list.PushBack(1);
    list.PushBack(2);
    list.PushBack(3);
    list.PushBack(4);
    list.PushBack(5);

    list.Insert(list.cbegin(), 6);
    list.Insert(list.cend(), 7);

    auto it_b = list.cbegin();
    std::advance(it_b, 3);
    list.Insert(it_b, 8);

    ASSERT_TRUE(CheckEqual({6, 1, 2, 8, 3, 4, 5, 7}, list));

    list.Erase(list.cbegin());
    list.Erase(std::prev(list.cend()));

    auto it_e = list.cend();
    std::advance(it_e, -3);
    list.Erase(it_e);

    ASSERT_TRUE(CheckEqual({1, 2, 8, 4, 5}, list));
}

TEST(List, IteratorAssertions) {
    std::ignore = CheckIter<typename List<int>::iterator, int>{};
    std::ignore = CheckIter<typename List<int>::const_iterator, const int>{};
    std::ignore = CheckIter<typename List<int>::reverse_iterator, int>{};
    std::ignore = CheckIter<typename List<int>::const_reverse_iterator, const int>{};

    static_assert(std::is_same_v<decltype(List<int>().begin()), List<int>::iterator>);
    static_assert(std::is_same_v<decltype(List<int>().cbegin()), List<int>::const_iterator>);

    static_assert(!std::is_assignable_v<List<int>::iterator, List<int>::const_iterator>);
    static_assert(std::is_assignable_v<List<int>::const_iterator, List<int>::iterator>);
}

TEST(List, Iterator) {
    List<int> l;
    l.PushBack(3);
    l.PushBack(4);
    l.PushBack(7);

    auto it = l.begin();
    ASSERT_EQ(it++, l.begin());
    ASSERT_EQ(*it, 4);
    ASSERT_EQ(--it, l.begin());

    it = l.end();
    ASSERT_EQ(it--, l.end());
    ASSERT_EQ(*it, 7);
    ASSERT_EQ(++it, l.end());
}

TEST(List, IteratorErase) {
    List<int> l;
    auto it = l.end();
    for (size_t i = 0; i < 10; ++i) {
        it = l.Insert(it, i);
    }
    ASSERT_EQ(*l.begin(), 9);
    ASSERT_EQ(*l.rbegin(), 0);

    for (size_t i = 0; i < 10; ++i) {
        l.Erase(it++);
    }
    ASSERT_TRUE(l.Empty());
}

TEST(List, Exceptions) {
    ASSERT_THROW(List<Counted<kSmallSize>>{kMediumSize}, CountedException);
    ASSERT_EQ(Counted<kSmallSize>::counter, 0);

    ASSERT_THROW(List<Explosive>{kMediumSize}, ExplosiveException);
    // See below.
    ASSERT_EQ(Explosive::exploded, false);

    {
        List<Explosive> explosive_list;
    }
    // See below.
    ASSERT_EQ(Explosive::exploded, false);

    {
        List<Explosive> guarded;
        auto safe = Explosive{Explosive::Safeguard{}};
        ASSERT_THROW(guarded.PushBack(safe), ExplosiveException);
    }

    // Destructor should not be called for an object with not finished constructor. The only
    // destructor called is for explosive with the safeguard.
    ASSERT_EQ(Explosive::exploded, false);
}

TEST(List, ExceptionalAllocator) {
    using DataT = size_t;
    using Alloc = ExceptionalAllocator<DataT>;

    auto exceptional_list = List<DataT, Alloc>(Alloc(kSmallSize));
    auto& list = exceptional_list;
    for (size_t i = 0; i < kSmallSize; ++i) {
        list.PushBack(i);
    }
    // Exactly kSmallSize allocations should have occured, no exceptions.

    auto try_modify = [&](auto callback, size_t expected_size) {
        ASSERT_THROW(callback(), CustomAllocatorException);
        ASSERT_EQ(list.Size(), expected_size);
        ASSERT_TRUE(std::equal(list.begin(), list.end(), IotaIterator<size_t>{0}));
    };

    try_modify([&] { list.PushBack({}); }, kSmallSize);
    try_modify([&] { list.PushFront({}); }, kSmallSize);
    try_modify(
        [&] {
            auto iter = list.begin();
            for (size_t i = 0; i < kSmallSize / 2; ++i) {
                ++iter;
            }
            list.Insert(iter, 0);
        },
        kSmallSize);

    while (!list.Empty()) {
        list.PopBack();
    }

    try_modify([&] { list.PushBack({}); }, 0);
    try_modify([&] { list.PushFront({}); }, 0);
}

TEST(List, SwapNonEmpty) {
    List<int> list1;
    list1.PushBack(1);
    list1.PushBack(2);

    List<int> list2(3, 5);

    list1.Swap(list2);

    ASSERT_TRUE(CheckEqual({5, 5, 5}, list1));
    ASSERT_TRUE(CheckEqual({1, 2}, list2));
}

TEST(List, SwapEmptyNonEmpty) {
    List<int> l1;
    l1.PushBack(10);
    l1.PushBack(20);

    List<int> l2;
    l1.Swap(l2);

    ASSERT_TRUE(l1.Empty());
    ASSERT_TRUE(CheckEqual({10, 20}, l2));
}

TEST(List, SwapIteratorStability) {
    List<int> l1;
    l1.PushBack(1);

    List<int> l2;
    l2.PushBack(2);

    auto it1 = l1.begin();
    auto it2 = l2.begin();
    l1.Swap(l2);

    ASSERT_EQ(*it1, 1);
    ASSERT_EQ(*it2, 2);
    ASSERT_EQ(l2.begin(), it1);
    ASSERT_EQ(l1.begin(), it2);
}

TEST(List, MultipleSwap) {
    List<int> list1(1'000'000, 0);
    List<int> list2(1'000'000, 1);

    for (int _ = 0; _ < 1'000'000; ++_) {
        list1.Swap(list2);
    }
}

TEST(List, SwapAlloc) {
    {
        using Alloc = PropagatableCountingAllocator<int, true, true>;

        List<int, Alloc> list1(Alloc{1});
        list1.PushBack(1);

        List<int, Alloc> list2(Alloc{2});
        list1.Swap(list2);

        ASSERT_TRUE(list1.Empty());
        ASSERT_TRUE(CheckEqual({1}, list2));

        ASSERT_EQ(list1.GetAllocator().value_, 2);
        ASSERT_EQ(list2.GetAllocator().value_, 1);
    }

    {
        using Alloc = PropagatableCountingAllocator<int, true, false>;

        List<int, Alloc> list1(Alloc{1});
        list1.PushBack(1);

        List<int, Alloc> list2(Alloc{2});
        list1.Swap(list2);

        ASSERT_TRUE(list1.Empty());
        ASSERT_TRUE(CheckEqual({1}, list2));

        ASSERT_EQ(list1.GetAllocator().value_, 1);
        ASSERT_EQ(list2.GetAllocator().value_, 2);
    }
}

template <typename T, bool PropagateCopy, bool PropagateSwap>
struct StateAllocator {
    using value_type = T;
    int id;

    StateAllocator(int i = 0) : id(i) {
    }

    template <typename U, bool PropagateCopyOther, bool PropagateSwapOther>
    StateAllocator(const StateAllocator<U, PropagateCopyOther, PropagateSwapOther>& other)
        : id(other.id) {
    }

    T* allocate(size_t n) {
        return std::allocator<T>().allocate(n);
    }

    void deallocate(T* p, size_t n) {
        std::allocator<T>().deallocate(p, n);
    }

    bool operator==(const StateAllocator& other) const {
        return id == other.id;
    }

    using propagate_on_container_copy_assignment = std::bool_constant<PropagateCopy>;
    using propagate_on_container_swap = std::bool_constant<PropagateSwap>;

    StateAllocator select_on_container_copy_construction() const {
        return StateAllocator(id);
    }

    template <typename U>
    struct rebind {
        using other = StateAllocator<U, PropagateCopy, PropagateSwap>;
    };
};

TEST(List, AllocatorPropagation) {
    // Copy Assignment
    {
        using AllocTrue = StateAllocator</*T = */ int,
                                         /*PCopy = */ true,
                                         /*PSwap = */ false>;
        List<int, AllocTrue> l1(AllocTrue(1)), l2(AllocTrue(2));
        l2.PushBack(10);
        l1 = l2;
        ASSERT_EQ(l1.GetAllocator().id, 2);
    }
    // Swap
    {
        using AllocTrue = StateAllocator<int, false, true>;
        List<int, AllocTrue> l1(AllocTrue(1)), l2(AllocTrue(2));
        l1.PushBack(100);
        l1.Swap(l2);
        ASSERT_EQ(l1.GetAllocator().id, 2);
        ASSERT_EQ(l2.GetAllocator().id, 1);
    }
}

TEST(ListComparison, UnequalAllocatorsDoNotAffectContentEquality) {
    {
        using Alloc = StateAllocator<int, false, false>;

        List<int, Alloc> lhs(Alloc(1));
        List<int, Alloc> rhs(Alloc(2));

        for (int x : {1, 2, 3, 4}) {
            lhs.PushBack(x);
            rhs.PushBack(x);
        }

        ASSERT_FALSE(lhs.GetAllocator() == rhs.GetAllocator());
        ASSERT_TRUE(lhs == rhs);
        ASSERT_FALSE(lhs != rhs);
    }

    {
        using Alloc = PropagatableCountingAllocator<int, false, false, false>;

        List<int, Alloc> lhs(Alloc{100});
        List<int, Alloc> rhs(Alloc{200});

        lhs.PushBack(10);
        lhs.PushBack(20);
        rhs.PushBack(10);
        rhs.PushBack(20);

        ASSERT_FALSE(lhs.GetAllocator() == rhs.GetAllocator());
        ASSERT_TRUE(lhs == rhs);
        ASSERT_FALSE(lhs != rhs);
    }

    {
        using Alloc = StateAllocator<int, false, false>;

        List<int, Alloc> lhs(Alloc(1));
        List<int, Alloc> rhs(Alloc(2));

        for (int x : {1, 2, 3}) {
            lhs.PushBack(x);
        }
        for (int x : {1, 2, 4}) {
            rhs.PushBack(x);
        }

        ASSERT_FALSE(lhs.GetAllocator() == rhs.GetAllocator());
        ASSERT_FALSE(lhs == rhs);
        ASSERT_TRUE(lhs != rhs);
    }
}

TEST(ListComparison, PoolAllocatorEqualityCases) {
    {
        PoolStorage<10'000> storage;
        using Alloc = PoolAllocator<int, 10'000>;

        List<int, Alloc> lhs(Alloc{storage});
        List<int, Alloc> rhs(Alloc{storage});

        for (int x : {1, 2, 3, 4, 5}) {
            lhs.PushBack(x);
            rhs.PushBack(x);
        }

        ASSERT_TRUE(lhs.GetAllocator() == rhs.GetAllocator());
        ASSERT_TRUE(lhs == rhs);
        ASSERT_FALSE(lhs != rhs);
    }

    {
        PoolStorage<10'000> storage1;
        PoolStorage<10'000> storage2;
        using Alloc = PoolAllocator<int, 10'000>;

        List<int, Alloc> lhs(Alloc{storage1});
        List<int, Alloc> rhs(Alloc{storage2});

        for (int x : {1, 2, 3, 4, 5}) {
            lhs.PushBack(x);
            rhs.PushBack(x);
        }

        ASSERT_FALSE(lhs.GetAllocator() == rhs.GetAllocator());
        ASSERT_TRUE(lhs == rhs);
        ASSERT_FALSE(lhs != rhs);
    }

    {
        PoolStorage<10'000> storage;
        using Alloc = PoolAllocator<int, 10'000>;

        List<int, Alloc> lhs(Alloc{storage});
        List<int, Alloc> rhs(Alloc{storage});

        for (int x : {1, 2, 3, 4, 5}) {
            lhs.PushBack(x);
        }
        for (int x : {1, 2, 3, 4, 6}) {
            rhs.PushBack(x);
        }

        ASSERT_TRUE(lhs.GetAllocator() == rhs.GetAllocator());
        ASSERT_FALSE(lhs == rhs);
        ASSERT_TRUE(lhs != rhs);
    }
}

TEST(List, PoolAllocator) {
    using DataT = size_t;
    using Alloc = PoolAllocator<DataT, kBigSize>;

    auto big_storage = PoolStorage<kBigSize>();
    auto big_list = List<DataT, Alloc>(Alloc(big_storage));
    for (size_t i = 0; i < kMediumSize; ++i) {
        big_list.PushBack(i);
    }

    ASSERT_TRUE(std::equal(big_list.begin(), big_list.end(), IotaIterator<DataT>{DataT(0)}));
    std::reverse(big_list.begin(), big_list.end());
    ASSERT_TRUE(std::equal(big_list.rbegin(), big_list.rend(), IotaIterator<DataT>{DataT(0)}));
}

TEST(List, EmptyAlloc) {
    using Alloc = PoolAllocator<size_t, kBigSize>;
    using EmptyAlloc = std::allocator<size_t>;

    ASSERT_TRUE(sizeof(List<size_t, EmptyAlloc>) < sizeof(List<size_t, Alloc>));
}

TEST(List, OutOfMemory) {
    using DataT = size_t;
    constexpr size_t kBytesCount = kSmallSize * (sizeof(DataT) + sizeof(void*) + sizeof(void*));
    using Alloc = PoolAllocator<DataT, kBytesCount>;

    auto small_storage = PoolStorage<kBytesCount>();
    auto small_list = List<DataT, Alloc>(Alloc(small_storage));
    for (size_t i = 0; i < kSmallSize; ++i) {
        small_list.PushFront(i);
    }

    ASSERT_THROW(small_list.PushBack({}), std::bad_alloc);
    ASSERT_EQ(small_list.Size(), kSmallSize);
    ASSERT_TRUE(std::equal(small_list.rbegin(), small_list.rend(), IotaIterator<DataT>{0}));

    ASSERT_THROW(small_list.PushFront({}), std::bad_alloc);
    ASSERT_EQ(small_list.Size(), kSmallSize);
    ASSERT_TRUE(std::equal(small_list.rbegin(), small_list.rend(), IotaIterator<DataT>{0}));

    // No allocations for empty list.
    std::ignore = List<DataT, Alloc>(Alloc(small_storage));

    {
        auto new_list = List<DataT, Alloc>(Alloc(small_storage));
        ASSERT_THROW(new_list.PushBack({}), std::bad_alloc);
    }

    // All allocated data is still valid
    ASSERT_EQ(small_list.Size(), kSmallSize);
    ASSERT_TRUE(std::equal(small_list.rbegin(), small_list.rend(), IotaIterator<DataT>{0}));
}

namespace by_mesyarik {

template <typename Alloc = std::allocator<int>>
void BasicList(Alloc alloc = Alloc()) {
    List<int, Alloc> lst(alloc);
    ASSERT_TRUE(lst.Empty());

    lst.PushBack(3);
    lst.PushBack(4);
    lst.PushFront(2);
    lst.PushBack(5);
    lst.PushFront(1);

    std::reverse(lst.begin(), lst.end());
    // now lst is 5 4 3 2 1

    ASSERT_EQ(lst.Size(), 5);
    ASSERT_TRUE(std::equal(lst.rbegin(), lst.rend(), IotaIterator<int>{1}));

    std::string s;
    for (int x : lst) {
        s += std::to_string(x);
    }
    ASSERT_EQ(s, "54321");
    // std::cerr << " check 1.1 ok, list contains 5 4 3 2 1" << std::endl;

    auto cit = lst.cbegin();
    std::advance(cit, 3);

    lst.Insert(cit, 6);
    lst.Insert(cit, 7);

    std::advance(cit, -3);
    lst.Insert(cit, 8);
    lst.Insert(cit, 9);
    // now lst is 5 4 8 9 3 6 7 2 1

    ASSERT_EQ(lst.Size(), 9);

    s.clear();
    for (int x : lst) {
        s += std::to_string(x);
    }
    ASSERT_EQ(s, "548936721");
    // std::cerr << " check 1.2 ok, list contains 5 4 8 9 3 6 7 2 1" << std::endl;

    lst.Erase(lst.cbegin());
    lst.Erase(cit);

    lst.PopFront();
    lst.PopBack();

    const auto copy = lst;
    ASSERT_EQ(lst.Size(), 5);
    ASSERT_EQ(copy.Size(), 5);
    // now both lists are 8 9 6 7 2

    s.clear();
    for (int x : lst) {
        s += std::to_string(x);
    }
    ASSERT_EQ(s, "89672");
    // std::cerr << " check 1.3 ok, list contains 8 9 6 7 2" << std::endl;

    auto rit = lst.rbegin();
    ++rit;
    lst.Erase(rit.base());
    ASSERT_EQ(lst.Size(), 4);

    rit = lst.rbegin();
    *rit = 3;

    // now lst: 8 9 6 3, copy: 8 9 6 7 2
    s.clear();
    for (int x : lst) {
        s += std::to_string(x);
    }
    ASSERT_EQ(s, "8963");

    ASSERT_EQ(copy.Size(), 5);

    s.clear();
    for (int x : copy) {
        s += std::to_string(x);
    }
    ASSERT_EQ(s, "89672");

    // std::cerr << " check 1.4 ok, list contains 8 9 6 3, another list is still 8 9 6 7 2" <<
    // std::endl;

    typename List<int, Alloc>::const_reverse_iterator crit = rit;
    crit = copy.rbegin();
    ASSERT_EQ(*crit, 2);

    cit = crit.base();
    std::advance(cit, -2);
    ASSERT_EQ(*cit, 7);
}

TEST(List, Basic) {
    {
        BasicList<>();
        ASSERT_FALSE(HasFailure());
    }

    {
        PoolStorage<200'000> storage;
        PoolAllocator<int, 200'000> alloc(storage);

        BasicList<PoolAllocator<int, 200'000>>(alloc);
        ASSERT_FALSE(HasFailure());
    }
}

struct VerySpecialType {
    int x = 0;

    explicit VerySpecialType(int x) : x(x) {
    }
};

struct NotDefaultConstructible {
    NotDefaultConstructible() = delete;

    NotDefaultConstructible(VerySpecialType x) : x(x) {
    }

    VerySpecialType x;
};

struct Accountant {
    // Some field of strange size
    char arr[40];

    static size_t ctor_calls;
    static size_t dtor_calls;

    static void Reset() {
        ctor_calls = 0;
        dtor_calls = 0;
    }

    Accountant() {
        ++ctor_calls;
    }

    Accountant(const Accountant&) {
        ++ctor_calls;
    }

    Accountant& operator=(const Accountant&) {
        // Actually, when it comes to assign one list to another,
        // list can use element-wise assignment instead of destroying nodes and creating new
        // ones
        ++ctor_calls;
        ++dtor_calls;
        return *this;
    }

    Accountant(Accountant&&) = delete;
    Accountant& operator=(Accountant&&) = delete;

    ~Accountant() {
        ++dtor_calls;
    }
};

size_t Accountant::ctor_calls = 0;
size_t Accountant::dtor_calls = 0;

template <typename Alloc = std::allocator<NotDefaultConstructible>>
void TestNotDefaultConstructible(Alloc alloc = Alloc()) {
    List<NotDefaultConstructible, Alloc> lst(alloc);
    ASSERT_TRUE(lst.Empty());

    lst.PushBack(VerySpecialType(0));
    ASSERT_EQ(lst.Size(), 1);
    ASSERT_FALSE(lst.Empty());

    lst.PopFront();
    ASSERT_TRUE(lst.Empty());
}

TEST(List, NotDefaultConstructible) {
    {
        TestNotDefaultConstructible<>();
        ASSERT_FALSE(HasFailure());
    }

    {
        PoolStorage<200'000> storage;
        PoolAllocator<int, 200'000> alloc(storage);

        TestNotDefaultConstructible<PoolAllocator<NotDefaultConstructible, 200'000>>(alloc);
        ASSERT_FALSE(HasFailure());
    }
}

struct AssertionFailure : std::exception {};

template <typename Alloc = std::allocator<Accountant>>
void TestAccountant(Alloc alloc = Alloc()) {
    Accountant::Reset();

    {
        List<Accountant, Alloc> lst(5, alloc);
        ASSERT_EQ(lst.Size(), 5);
        ASSERT_EQ(Accountant::ctor_calls, 5);

        List<Accountant, Alloc> another = lst;
        ASSERT_EQ(another.Size(), 5);
        ASSERT_EQ(Accountant::ctor_calls, 10);
        ASSERT_EQ(Accountant::dtor_calls, 0);

        another.PopBack();
        another.PopFront();
        ASSERT_EQ(Accountant::dtor_calls, 2);

        lst = another;  // dtor_calls += 5, ctor_calls += 3
        ASSERT_EQ(another.Size(), 3);
        ASSERT_EQ(lst.Size(), 3);

        ASSERT_EQ(Accountant::ctor_calls, 13);
        ASSERT_EQ(Accountant::dtor_calls, 7);

    }  // dtor_calls += 6

    ASSERT_EQ(Accountant::ctor_calls, 13);
    ASSERT_EQ(Accountant::dtor_calls, 13);
}

struct ComparableAccountant : Accountant {
    ComparableAccountant() : Accountant() {
    }
};

bool operator==(const ComparableAccountant&, const ComparableAccountant&) {
    return true;
}

TEST(ListComparison, EqualityDoesNotCopyAllocateOrDestroyAccountants) {
    using Alloc = PropagatableCountingAllocator<ComparableAccountant, false, false, true>;

    Accountant::Reset();

    List<ComparableAccountant, Alloc> lhs(Alloc{1});
    List<ComparableAccountant, Alloc> rhs(Alloc{1});

    for (size_t i = 0; i < 5; ++i) {
        lhs.PushBack(ComparableAccountant{});
        rhs.PushBack(ComparableAccountant{});
    }

    const size_t ctor_calls_before_prepare = Accountant::ctor_calls;
    const size_t dtor_calls_before_prepare = Accountant::dtor_calls;
    const int lhs_allocations_before_prepare = lhs.GetAllocator().allocations_;
    const int rhs_allocations_before_prepare = rhs.GetAllocator().allocations_;
    const int lhs_deallocations_before_prepare = lhs.GetAllocator().deallocations_;
    const int rhs_deallocations_before_prepare = rhs.GetAllocator().deallocations_;

    ASSERT_TRUE(lhs == rhs);

    ASSERT_EQ(Accountant::ctor_calls, ctor_calls_before_prepare);
    ASSERT_EQ(Accountant::dtor_calls, dtor_calls_before_prepare);
    ASSERT_EQ(lhs.GetAllocator().allocations_, lhs_allocations_before_prepare);
    ASSERT_EQ(rhs.GetAllocator().allocations_, rhs_allocations_before_prepare);
    ASSERT_EQ(lhs.GetAllocator().deallocations_, lhs_deallocations_before_prepare);
    ASSERT_EQ(rhs.GetAllocator().deallocations_, rhs_deallocations_before_prepare);

    ASSERT_FALSE(lhs != rhs);

    ASSERT_EQ(Accountant::ctor_calls, ctor_calls_before_prepare);
    ASSERT_EQ(Accountant::dtor_calls, dtor_calls_before_prepare);
    ASSERT_EQ(lhs.GetAllocator().allocations_, lhs_allocations_before_prepare);
    ASSERT_EQ(rhs.GetAllocator().allocations_, rhs_allocations_before_prepare);
    ASSERT_EQ(lhs.GetAllocator().deallocations_, lhs_deallocations_before_prepare);
    ASSERT_EQ(rhs.GetAllocator().deallocations_, rhs_deallocations_before_prepare);
}

struct EmplaceCounter {
    static int direct_ctor_calls;
    static int copy_ctor_calls;
    static int dtor_calls;

    static void Reset() {
        direct_ctor_calls = 0;
        copy_ctor_calls = 0;
        dtor_calls = 0;
    }

    EmplaceCounter(int a, int b, int c, int d, int e) : a(a), b(b), c(c), d(d), e(e) {
        ++direct_ctor_calls;
    }

    EmplaceCounter(const EmplaceCounter& other)
        : a(other.a), b(other.b), c(other.c), d(other.d), e(other.e) {
        ++copy_ctor_calls;
    }

    EmplaceCounter& operator=(const EmplaceCounter&) = default;

    ~EmplaceCounter() {
        ++dtor_calls;
    }

    int a;
    int b;
    int c;
    int d;
    int e;
};

int EmplaceCounter::direct_ctor_calls = 0;
int EmplaceCounter::copy_ctor_calls = 0;
int EmplaceCounter::dtor_calls = 0;

TEST(List, EmplaceBackConstructsElementInPlace) {
    EmplaceCounter::Reset();

    {
        List<EmplaceCounter> list;
        list.EmplaceBack(1, 2, 3, 4, 5);

        ASSERT_EQ(list.Size(), 1);
        ASSERT_EQ(EmplaceCounter::direct_ctor_calls, 1);
        ASSERT_EQ(EmplaceCounter::copy_ctor_calls, 0);

        const auto& value = *list.begin();
        ASSERT_EQ(value.a, 1);
        ASSERT_EQ(value.b, 2);
        ASSERT_EQ(value.c, 3);
        ASSERT_EQ(value.d, 4);
        ASSERT_EQ(value.e, 5);

        ASSERT_EQ(EmplaceCounter::direct_ctor_calls, 1);
        ASSERT_EQ(EmplaceCounter::copy_ctor_calls, 0);
        ASSERT_EQ(EmplaceCounter::dtor_calls, 0);
    }

    ASSERT_EQ(EmplaceCounter::direct_ctor_calls, 1);
    ASSERT_EQ(EmplaceCounter::copy_ctor_calls, 0);
    ASSERT_EQ(EmplaceCounter::dtor_calls, 1);
}

TEST(List, EmplaceFrontConstructsElementInPlace) {
    EmplaceCounter::Reset();

    {
        List<EmplaceCounter> list;
        list.EmplaceFront(1, 2, 3, 4, 5);

        ASSERT_EQ(list.Size(), 1);
        ASSERT_EQ(EmplaceCounter::direct_ctor_calls, 1);
        ASSERT_EQ(EmplaceCounter::copy_ctor_calls, 0);

        const auto& value = *list.begin();
        ASSERT_EQ(value.a, 1);
        ASSERT_EQ(value.b, 2);
        ASSERT_EQ(value.c, 3);
        ASSERT_EQ(value.d, 4);
        ASSERT_EQ(value.e, 5);

        ASSERT_EQ(EmplaceCounter::direct_ctor_calls, 1);
        ASSERT_EQ(EmplaceCounter::copy_ctor_calls, 0);
        ASSERT_EQ(EmplaceCounter::dtor_calls, 0);
    }

    ASSERT_EQ(EmplaceCounter::direct_ctor_calls, 1);
    ASSERT_EQ(EmplaceCounter::copy_ctor_calls, 0);
    ASSERT_EQ(EmplaceCounter::dtor_calls, 1);
}

TEST(List, Accountant) {
    {
        TestAccountant<>();
        ASSERT_FALSE(HasFailure());
    }

    {
        PoolStorage<200'000> storage;
        PoolAllocator<int, 200'000> alloc(storage);

        TestAccountant<PoolAllocator<Accountant, 200'000>>(alloc);
        ASSERT_FALSE(HasFailure());
    }
}

struct AccountantException : std::exception {
    AccountantException(const char* msg) : msg(msg) {
    }

    const char* what() const noexcept override {
        return msg;
    }

    const char* msg;
};

struct ThrowingAccountant : public Accountant {
    static bool need_throw;

    int value = 0;

    ThrowingAccountant(int value = 0) : Accountant(), value(value) {
        if (need_throw && ctor_calls % 5 == 4) {
            throw AccountantException("Ahahahaha you have been cocknut");
        }
    }

    ThrowingAccountant(const ThrowingAccountant& other) : Accountant(), value(other.value) {
        if (need_throw && ctor_calls % 5 == 4) {
            throw AccountantException("Ahahahaha you have been cocknut");
        }
    }

    ThrowingAccountant& operator=(const ThrowingAccountant& other) {
        value = other.value;
        ++ctor_calls;
        ++dtor_calls;
        if (need_throw && ctor_calls % 5 == 4) {
            throw AccountantException("Ahahahaha you have been cocknut");
        }
        return *this;
    }
};

bool ThrowingAccountant::need_throw = false;

TEST(List, ExceptionSafety) {
    Accountant::Reset();

    ThrowingAccountant::need_throw = true;

    ASSERT_THROW(List<ThrowingAccountant> lst{8}, AccountantException);
    ASSERT_EQ(Accountant::ctor_calls, 4);
    ASSERT_EQ(Accountant::dtor_calls, 4);

    ThrowingAccountant::need_throw = false;
    List<ThrowingAccountant> lst{8};

    List<ThrowingAccountant> lst2;
    for (int i = 0; i < 13; ++i) {
        lst2.PushBack(i);
    }

    Accountant::Reset();
    ThrowingAccountant::need_throw = true;

    ASSERT_THROW(List<ThrowingAccountant>{lst2}, AccountantException);
    ASSERT_EQ(Accountant::ctor_calls, 4);
    ASSERT_EQ(Accountant::dtor_calls, 4);

    Accountant::Reset();

    ASSERT_THROW(lst = lst2, AccountantException);
    ASSERT_EQ(Accountant::ctor_calls, 4);
    ASSERT_EQ(Accountant::dtor_calls, 4);

    // Actually it may not be 8 (although de facto it is), but the only thing we can demand here
    // is the abscence of memory leaks
    //
    // ASSERT_EQ(lst.size(), 8);
}

TEST(List, EmplaceExceptionSafety) {
    {
        Accountant::Reset();
        ThrowingAccountant::need_throw = true;

        List<ThrowingAccountant> list;
        list.EmplaceBack(1);
        list.EmplaceBack(2);
        list.EmplaceBack(3);

        ASSERT_THROW(list.EmplaceBack(4), AccountantException);

        ASSERT_EQ(list.Size(), 3);

        auto it = list.begin();
        ASSERT_EQ(it->value, 1);
        ++it;
        ASSERT_EQ(it->value, 2);
        ++it;
        ASSERT_EQ(it->value, 3);

        ASSERT_EQ(Accountant::ctor_calls, 4);
        ASSERT_EQ(Accountant::dtor_calls, 1);

        ThrowingAccountant::need_throw = false;
        Accountant::Reset();
    }
    {
        Accountant::Reset();
        ThrowingAccountant::need_throw = true;

        List<ThrowingAccountant> list;
        list.EmplaceBack(1);
        list.EmplaceBack(2);
        list.EmplaceBack(3);

        ASSERT_THROW(list.EmplaceFront(4), AccountantException);

        ASSERT_EQ(list.Size(), 3);

        auto it = list.begin();
        ASSERT_EQ(it->value, 1);
        ++it;
        ASSERT_EQ(it->value, 2);
        ++it;
        ASSERT_EQ(it->value, 3);

        ASSERT_EQ(Accountant::ctor_calls, 4);
        ASSERT_EQ(Accountant::dtor_calls, 1);

        ThrowingAccountant::need_throw = false;
        Accountant::Reset();
    }
}

TEST(List, WhimsicalAllocator) {
    {
        List<int, WhimsicalAllocator<int, true, true>> lst;

        lst.PushBack(1);
        lst.PushBack(2);

        auto copy = lst;
        ASSERT_NE(copy.GetAllocator(), lst.GetAllocator());

        lst = copy;
        ASSERT_EQ(copy.GetAllocator(), lst.GetAllocator());
    }
    {
        List<int, WhimsicalAllocator<int, false, false>> lst;

        lst.PushBack(1);
        lst.PushBack(2);

        auto copy = lst;
        ASSERT_EQ(copy.GetAllocator(), lst.GetAllocator());

        lst = copy;
        ASSERT_EQ(copy.GetAllocator(), lst.GetAllocator());
    }
    {
        List<int, WhimsicalAllocator<int, true, false>> lst;

        lst.PushBack(1);
        lst.PushBack(2);

        auto copy = lst;
        ASSERT_NE(copy.GetAllocator(), lst.GetAllocator());

        lst = copy;
        ASSERT_NE(copy.GetAllocator(), lst.GetAllocator());
    }
    {
        List<Counted<4>, WhimsicalAllocator<Counted<4>, true, true>> lst;  // T, construct, assign
        auto counter = Counted<4>();
        auto alloc = lst.GetAllocator();

        lst.PushBack(counter);
        lst.PushBack(counter);

        try {
            lst.PushBack(counter);
        } catch (...) {
            ASSERT_EQ(alloc, lst.GetAllocator());
        }

        auto lst_copy = List<Counted<4>, WhimsicalAllocator<Counted<4>, true, true>>();
        alloc = lst_copy.GetAllocator();
        try {
            lst_copy = lst;
        } catch (...) {
            ASSERT_NE(lst.GetAllocator(), lst_copy.GetAllocator());
            ASSERT_TRUE(lst_copy.GetAllocator() == alloc);
        }
    }
    {
        List<Counted<4>, WhimsicalAllocator<Counted<4>, false, true>> lst;
        auto counter = Counted<4>();
        auto alloc = lst.GetAllocator();

        lst.PushBack(counter);
        lst.PushBack(counter);

        try {
            lst.PushBack(counter);
        } catch (...) {
            ASSERT_EQ(alloc, lst.GetAllocator());
        }
    }
    {
        List<Counted<4>, WhimsicalAllocator<Counted<4>, true, false>> lst;
        auto counter = Counted<4>();
        auto alloc = lst.GetAllocator();

        lst.PushBack(counter);
        lst.PushBack(counter);

        try {
            lst.PushBack(counter);
        } catch (...) {
            ASSERT_EQ(alloc, lst.GetAllocator());
        }
    }
    {
        List<Counted<4>, WhimsicalAllocator<Counted<4>, false, false>> lst;
        auto counter = Counted<4>();
        auto alloc = lst.GetAllocator();

        lst.PushBack(counter);
        lst.PushBack(counter);

        try {
            lst.PushBack(counter);
        } catch (...) {
            ASSERT_EQ(alloc, lst.GetAllocator());
        }
    }
}

TEST(List, Big) {
    using Alloc = PoolAllocator<char, kStorageSize>;
    Alloc alloc{static_storage};

    std::deque<char, Alloc> d(alloc);

    d.push_back(1);
    ASSERT_EQ(d.back(), 1);

    d.resize(2'500'000, 5);
    ASSERT_EQ(d[1'000'000], 5);

    d.pop_back();
    for (int i = 0; i < 2'000'000; ++i) {
        d.push_back(i % 100);
    }

    ASSERT_EQ(d.size(), 4'499'999);
    ASSERT_EQ(d[4'000'000], 1);

    for (int i = 0; i < 4'000'000; ++i) {
        d.pop_front();
    }

    ASSERT_EQ(d[400'000], 1);
}

// Finally, some tricky fixtures to test custom allocator.
// Done by professional, don't try to repeat
class Chaste {
private:
    int x_ = 0;
    Chaste() = default;

    Chaste(int x) : x_(x) {
    }

    // Nobody can construct me except this guy
    template <typename T>
    friend struct TheChosenOne;

public:
    Chaste(const Chaste&) = default;
    Chaste(Chaste&&) = default;

    bool operator==(const Chaste& other) const {
        return x_ == other.x_;
    }
};

template <typename T>
struct TheChosenOne : public std::allocator<T> {
    TheChosenOne() {
    }

    template <typename U>
    TheChosenOne(const TheChosenOne<U>&) {
    }

    template <typename... Args>
    void construct(T* p, Args&&... args) const {
        new (p) T(std::forward<Args>(args)...);
    }

    void construct(std::pair<const Chaste, Chaste>* p, int a, int b) const {
        new (p) std::pair<const Chaste, Chaste>(Chaste(a), Chaste(b));
    }

    void destroy(T* p) const {
        p->~T();
    }

    template <typename U>
    struct rebind {
        using other = TheChosenOne<U>;
    };
};

TEST(List, CustomAlloc) {
    List<Chaste, TheChosenOne<Chaste>> m;  // this letter will say a lot about our society

    m.EmplaceBack(0);

    {
        auto mm = m;
        mm.Erase(mm.begin());
    }

    for (int i = 0; i < 1'000'000; ++i) {
        m.EmplaceBack(i);
    }

    for (int i = 0; i < 500'000; ++i) {
        auto it = m.begin();
        ++it, ++it;
        m.Erase(it);
    }
}

TEST(List, EmplaceBackWithCustomAllocatorConstruct) {
    List<Chaste, TheChosenOne<Chaste>> list;

    list.EmplaceBack(42);

    ASSERT_EQ(list.Size(), 1);

    auto copy = list;
    ASSERT_EQ(copy.Size(), 1);
}

TEST(List, EmplaceFrontWithCustomAllocatorConstruct) {
    List<Chaste, TheChosenOne<Chaste>> list;

    list.EmplaceFront(42);

    ASSERT_EQ(list.Size(), 1);

    auto copy = list;
    ASSERT_EQ(copy.Size(), 1);
}

TEST(List, EmplaceBackManyWithCustomAllocatorConstruct) {
    List<Chaste, TheChosenOne<Chaste>> list;

    list.EmplaceBack(1);
    list.EmplaceBack(2);
    list.EmplaceBack(3);

    ASSERT_EQ(list.Size(), 3);

    auto copy = list;
    ASSERT_EQ(copy.Size(), 3);
}

TEST(List, EmplaceFrontManyWithCustomAllocatorConstruct) {
    List<Chaste, TheChosenOne<Chaste>> list;

    list.EmplaceFront(1);
    list.EmplaceFront(2);
    list.EmplaceFront(3);

    ASSERT_EQ(list.Size(), 3);

    auto copy = list;
    ASSERT_EQ(copy.Size(), 3);
}

TEST(List, ReferenceStability) {
    List<int> my_list;
    my_list.PushBack(100);
    int& ref = *my_list.begin();

    ASSERT_EQ(ref, 100);

    for (int i = 0; i < 1000; ++i) {
        my_list.PushBack(i);
        my_list.PushFront(i * 2);
    }

    ASSERT_EQ(ref, 100);

    ref = 500;
    bool found = false;
    for (int val : my_list) {
        if (val == 500) {
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found);
    ASSERT_EQ(ref, 500);

    auto it = my_list.begin();
    while (*it != 500) {
        ++it;
    }

    auto prev_it = std::prev(it);
    auto next_it = std::next(it);

    my_list.Erase(prev_it);
    my_list.Erase(next_it);

    ASSERT_EQ(ref, 500);
}

}  // namespace by_mesyarik
