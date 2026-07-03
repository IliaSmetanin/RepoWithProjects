#include "deque.hpp"

#include "gtest/gtest.h"

#include <algorithm>
#include <cassert>
#include <deque>
#include <numeric>
#include <random>
#include <stdexcept>
#include <type_traits>

#include <gtest/gtest.h>

namespace testsuite1 {
// By Mesyarik

TEST(Deque, Basic) {
    Deque<int> d(10, 3);

    d[3] = 5;
    d[7] = 8;
    d[9] = 10;

    std::string s = "33353338310";
    std::string ss;
    Deque<int> dd;

    {
        Deque<int> d2 = d;
        dd = d2;
    }

    d[1] = 2;
    d.At(2) = 1;

    ASSERT_THROW(d.At(10) = 0, std::out_of_range);

    const Deque<int>& ddd = dd;
    for (size_t i = 0; i < ddd.Size(); ++i) {
        ss += std::to_string(ddd[i]);
    }

    ASSERT_EQ(s, ss);
}

TEST(Deque, PushAndPop) {
    Deque<int> d(1);

    d[0] = 0;

    for (int i = 0; i < 8; ++i) {
        d.PushBack(i);
        d.PushFront(i);
    }

    for (int i = 0; i < 12; ++i) {
        d.PopFront();
    }

    d.PopBack();
    ASSERT_EQ(d.Size(), 4);

    std::string ss;

    for (size_t i = 0; i < d.Size(); ++i) {
        ss += std::to_string(d[i]);
    }

    ASSERT_EQ(ss, "3456");
}

TEST(Deque, PushAndPopWithIterators) {
    Deque<int> d;

    for (int i = 0; i < 1000; ++i) {
        for (int j = 0; j < 1000; ++j) {
            if (j % 3 == 2) {
                d.PopBack();
            } else {
                d.PushFront(i * j);
            }
        }
    }

    ASSERT_EQ(d.Size(), 334'000);

    Deque<int>::iterator left = d.begin() + 100'000;
    Deque<int>::iterator right = d.end() - 233'990;
    size_t fr_count = 0;
    size_t b_count = 0;
    while (d.begin() != left) {
        d.PopFront();
        ++fr_count;
    }
    while (d.end() != right) {
        d.PopBack();
        ++b_count;
    }

    ASSERT_EQ(d.Size(), 10);

    ASSERT_EQ(right - left, 10);

    std::string s;
    for (auto it = left; it != right; ++it) {
        ++*it;
    }
    for (auto it = right - 1; it >= left; --it) {
        s += std::to_string(*it);
    }

    ASSERT_EQ(s, "51001518515355154401561015695158651595016120162051");
}

struct S {
    int x = 0;
    double y = 0.0;
};

TEST(Deque, Iterators) {
    Deque<S> d(5, {1, 2.0});
    const Deque<S>& cd = d;

    static_assert(!std::is_assignable_v<decltype(*cd.begin()), S>);
    static_assert(std::is_assignable_v<decltype(*d.begin()), S>);
    static_assert(!std::is_assignable_v<decltype(*d.cbegin()), S>);

    static_assert(!std::is_assignable_v<decltype(*cd.end()), S>);
    static_assert(std::is_assignable_v<decltype(*d.end()), S>);
    static_assert(!std::is_assignable_v<decltype(*d.cend()), S>);

    ASSERT_EQ(cd.Size(), 5);

    auto it = d.begin() + 2;
    auto cit = cd.end() - 3;

    it->x = 5;
    ASSERT_EQ(cit->x, 5);

    d.Erase(d.begin() + 1);
    d.Erase(d.begin() + 3);
    ASSERT_EQ(d.Size(), 3);

    auto dd = cd;

    dd.PopBack();
    dd.Insert(dd.begin(), {3, 4.0});
    dd.Insert(dd.begin() + 2, {4, 5.0});

    std::string s;
    for (const auto& x : dd) {
        s += std::to_string(x.x);
    }
    ASSERT_EQ(s, "3145");

    std::string ss;
    for (const auto& x : d) {
        ss += std::to_string(x.x);
    }
    ASSERT_EQ(ss, "151");
}

TEST(Deque, PushStress) {
    Deque<int> d;

    d.PushBack(1);
    d.PushFront(2);

    auto left_ptr = &*d.begin();
    auto right_ptr = &*(d.end() - 1);

    d.PushBack(3);
    d.PushFront(4);
    auto left = *d.begin();
    auto right = *(d.end() - 1);

    for (int i = 0; i < 10'000; ++i) {
        d.PushBack(i);
    }
    for (int i = 0; i < 20'000; ++i) {
        d.PushFront(i);
    }

    std::string s;
    s += std::to_string(left);
    s += std::to_string(right);

    s += std::to_string(*left_ptr);
    s += std::to_string(*right_ptr);
    // for (auto it = left; it <= right; ++it) {
    //     s += std::to_string(*it);
    // }
    ASSERT_EQ(s, "4321");
}

struct VerySpecialType {
    int x = 0;

    explicit VerySpecialType(int x) : x(x) {
    }
};

struct NotDefaultConstructible {
    NotDefaultConstructible() = delete;
    NotDefaultConstructible(const NotDefaultConstructible&) = default;
    NotDefaultConstructible& operator=(const NotDefaultConstructible&) = default;

    NotDefaultConstructible(VerySpecialType v) : x(v.x) {
    }

public:
    int x = 0;
};

TEST(Deque, SpecialType) {
    Deque<NotDefaultConstructible> d;

    NotDefaultConstructible ndc = VerySpecialType(-1);

    for (int i = 0; i < 1500; ++i) {
        ++ndc.x;
        d.PushBack(ndc);
    }

    ASSERT_EQ(d.Size(), 1500);

    for (int i = 0; i < 1300; ++i) {
        d.PopFront();
    }

    ASSERT_EQ(d.Size(), 200);

    ASSERT_EQ(d[99].x, 1399);

    d[100] = VerySpecialType(0);
    ASSERT_EQ(d[100].x, 0);
}

struct Explosive {
    int x = 0;

    Explosive(int x) : x(x) {
    }

    Explosive(const Explosive&) {
        if (x) {
            throw std::runtime_error("Boom!");
        }
    }

    Explosive& operator=(const Explosive&) = default;
};

TEST(Deque, ExceptionSafetyStress) {
    Deque<Explosive> d;
    d.PushBack(Explosive(0));

    for (int i = 0; i < 30'000; ++i) {
        auto it = d.begin();
        auto x = it->x;
        size_t sz = d.Size();
        try {
            if (i % 2) {
                d.PushBack(Explosive(1));
            } else {
                d.PushFront(Explosive(1));
            }
        } catch (...) {
            ASSERT_EQ(it, d.begin());
            ASSERT_EQ(d.begin()->x, x);
            ASSERT_EQ(d.Size(), sz);
        }

        d.PushBack(Explosive(0));
    }
}

TEST(Deque, Cheats) {
    static_assert(!std::is_same_v<std::deque<VerySpecialType>, Deque<VerySpecialType>>,
                  "You cannot use std::deque, cheater!");
    static_assert(!std::is_base_of_v<std::deque<VerySpecialType>, Deque<VerySpecialType>>,
                  "You cannot use std::deque, cheater!");
}

}  // namespace testsuite1

namespace testsuite2 {
// By Unrealf1

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
            throw 2;
        }
        return *this;
    }

    int durability;
    int data;

private:
    Fragile() {
    }
};

struct ExplosiveException {};

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

    // TODO: is this ok..?
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
    DefaultConstructible() {
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

struct CountedException : public std::exception {};

template <int when_throw>
struct Counted {
    inline static int counter = 0;

    Counted() {
        ++counter;
        if (counter == when_throw) {
            --counter;
            throw CountedException();
        }
    }

    Counted(const Counted&) : Counted() {
    }

    ~Counted() {
        --counter;
    }
};

template <typename Iter, typename T>
struct CheckIter {
    using Traits = std::iterator_traits<Iter>;

    static_assert(
        std::is_same_v<std::remove_cv_t<typename Traits::value_type>, std::remove_cv_t<T>>);
    static_assert(std::is_same_v<typename Traits::pointer, T*>);
    static_assert(std::is_same_v<typename Traits::reference, T&>);
    static_assert(
        std::is_same_v<typename Traits::iterator_category, std::random_access_iterator_tag>);

    static_assert(std::is_same_v<decltype(std::declval<Iter>()++), Iter>);
    static_assert(std::is_same_v<decltype(++std::declval<Iter>()), Iter&>);
    static_assert(std::is_same_v<decltype(std::declval<Iter>() + 5), Iter>);
    static_assert(std::is_same_v<decltype(5 + std::declval<Iter>()), Iter>);
    static_assert(std::is_same_v<decltype(std::declval<Iter>() += 5), Iter&>);

    static_assert(std::is_same_v<decltype(std::declval<Iter>() - std::declval<Iter>()),
                                 typename Traits::difference_type>);
    static_assert(std::is_same_v<decltype(*std::declval<Iter>()), T&>);

    static_assert(std::is_same_v<decltype(std::declval<Iter>() < std::declval<Iter>()), bool>);
    static_assert(std::is_same_v<decltype(std::declval<Iter>() <= std::declval<Iter>()), bool>);
    static_assert(std::is_same_v<decltype(std::declval<Iter>() > std::declval<Iter>()), bool>);
    static_assert(std::is_same_v<decltype(std::declval<Iter>() >= std::declval<Iter>()), bool>);
    static_assert(std::is_same_v<decltype(std::declval<Iter>() == std::declval<Iter>()), bool>);
    static_assert(std::is_same_v<decltype(std::declval<Iter>() != std::declval<Iter>()), bool>);
};

TEST(Deque, DefaultConstructor) {
    Deque<int> defaulted;
    ASSERT_EQ(defaulted.Size(), 0);

    Deque<NotDefaultConstructible> without_default;
    ASSERT_EQ(without_default.Size(), 0);
}

TEST(Deque, CopyConstructor) {
    Deque<NotDefaultConstructible> without_default;
    Deque<NotDefaultConstructible> copy = without_default;
    ASSERT_EQ(copy.Size(), 0);
}

TEST(Deque, Size) {
    int size = 17;
    int value = 14;
    Deque<int> simple(size);
    ASSERT_EQ(simple.Size(), size_t(size));
    ASSERT_TRUE(std::all_of(simple.begin(), simple.end(), [](int item) { return item == 0; }));

    Deque<NotDefaultConstructible> less_simple(size, value);
    ASSERT_EQ(less_simple.Size(), size_t(size));
    ASSERT_TRUE(std::all_of(less_simple.begin(), less_simple.end(),
                            [&](const auto& item) { return item.data == value; }));

    Deque<DefaultConstructible> default_constructor(size);
    ASSERT_TRUE(std::all_of(
        default_constructor.begin(), default_constructor.end(),
        [](const auto& item) { return item.data == DefaultConstructible::kDefaultData; }));
}

TEST(Deque, Assign) {
    Deque<int> first(10, 10);
    Deque<int> second(9, 9);
    first = second;

    ASSERT_EQ(first.Size(), second.Size());
    ASSERT_EQ(first.Size(), 9);
    ASSERT_TRUE(std::equal(first.begin(), first.end(), second.begin()));
}

TEST(Deque, StaticAssertions) {
    using T1 = int;
    using T2 = NotDefaultConstructible;

    static_assert(std::is_default_constructible_v<Deque<T1>>, "should have default constructor");
    static_assert(std::is_default_constructible_v<Deque<T2>>, "should have default constructor");
    static_assert(std::is_copy_constructible_v<Deque<T1>>, "should have copy constructor");
    static_assert(std::is_copy_constructible_v<Deque<T2>>, "should have copy constructor");
    // static_assert(std::is_constructible_v<Deque<T1>, int>, "should have constructor from int");
    // static_assert(std::is_constructible_v<Deque<T2>, int>, "should have constructor from int");
    static_assert(std::is_constructible_v<Deque<T1>, int, const T1&>,
                  "should have constructor from int and const T&");
    static_assert(std::is_constructible_v<Deque<T2>, int, const T2&>,
                  "should have constructor from int and const T&");

    static_assert(std::is_copy_assignable_v<Deque<T1>>, "should have assignment operator");
    static_assert(std::is_copy_assignable_v<Deque<T2>>, "should have assignment operator");
}

TEST(Deque, IndexAccess) {
    Deque<size_t> defaulted(1300, 43);
    ASSERT_EQ(defaulted[0], defaulted[1280]);
    ASSERT_EQ(defaulted[0], 43);

    ASSERT_EQ(defaulted.At(0), defaulted[1280]);
    ASSERT_EQ(defaulted.At(0), 43);

    ASSERT_THROW(defaulted.At(size_t(-1)), std::out_of_range);
    ASSERT_THROW(defaulted.At(1300), std::out_of_range);
}

TEST(Deque, StaticAssertionsAccess) {
    Deque<size_t> defaulted;
    const Deque<size_t> constant;
    static_assert(std::is_same_v<decltype(defaulted[0]), size_t&>);
    static_assert(std::is_same_v<decltype(defaulted.At(0)), size_t&>);
    static_assert(std::is_same_v<decltype(constant[0]), const size_t&>);
    static_assert(std::is_same_v<decltype(constant.At(0)), const size_t&>);

    // static_assert(noexcept(defaulted[0]), "operator[] should not throw");
    static_assert(!noexcept(defaulted.At(0)), "at() can throw");
}

TEST(Deque, Swap) {
    Deque<int> first(3, 2);
    Deque<int> second(2, 1);

    auto& ref1 = first[0];
    auto it1 = first.begin();

    auto& ref2 = second[0];
    auto it2 = second.begin();

    first.Swap(second);

    ASSERT_EQ(first.Size(), 2);
    ASSERT_TRUE(std::all_of(first.begin(), first.end(), [](int v) { return v == 1; }));
    ASSERT_EQ(*it1, 2);
    ASSERT_EQ(ref1, 2);

    ASSERT_EQ(second.Size(), 3);
    ASSERT_TRUE(std::all_of(second.begin(), second.end(), [](int v) { return v == 2; }));
    ASSERT_EQ(*it2, 1);
    ASSERT_EQ(ref2, 1);
}

TEST(Deque, MultipleSwap) {
    Deque<int> first(1'000'000, 1);
    Deque<int> second(1'000'000, 2);

    constexpr int kIters = 999'999;

    for (int i = 0; i < kIters; ++i) {
        first.Swap(second);
    }

    ASSERT_EQ(first.Size(), 1'000'000);
    ASSERT_TRUE(std::all_of(first.begin(), first.end(), [](int v) { return v == 2; }));

    ASSERT_EQ(second.Size(), 1'000'000);
    ASSERT_TRUE(std::all_of(second.begin(), second.end(), [](int v) { return v == 1; }));
}

TEST(Deque, StaticAssertionsIterator) {
    CheckIter<Deque<int>::iterator, int> iter;
    std::ignore = iter;
    CheckIter<decltype(std::declval<Deque<int>>().rbegin()), int> reverse_iter;
    std::ignore = reverse_iter;
    CheckIter<decltype(std::declval<Deque<int>>().cbegin()), const int> const_iter;
    std::ignore = const_iter;

    static_assert(std::is_convertible_v<decltype(std::declval<Deque<int>>().begin()),
                                        decltype(std::declval<Deque<int>>().cbegin())>,
                  "should be able to construct const iterator from non const iterator");
    static_assert(!std::is_convertible_v<decltype(std::declval<Deque<int>>().cbegin()),
                                         decltype(std::declval<Deque<int>>().begin())>,
                  "should NOT be able to construct iterator from const iterator");
}

TEST(Deque, IteratorArithmetics) {
    Deque<int> empty;
    ASSERT_EQ(empty.end() - empty.begin(), 0);
    ASSERT_EQ(empty.begin() + 0, empty.end());
    ASSERT_EQ(0 + empty.begin(), empty.end());
    ASSERT_EQ(empty.end() - 0, empty.begin());

    Deque<int> one(1);
    auto iter2 = one.end();
    ASSERT_EQ(--iter2, one.begin());

    ASSERT_EQ(empty.rend() - empty.rbegin(), 0);
    ASSERT_EQ(empty.rbegin() + 0, empty.rend());
    ASSERT_EQ(0 + empty.rbegin(), empty.rend());
    ASSERT_EQ(empty.rend() - 0, empty.rbegin());

    ASSERT_EQ(empty.cend() - empty.cbegin(), 0);
    ASSERT_EQ(empty.cbegin() + 0, empty.cend());
    ASSERT_EQ(0 + empty.cbegin(), empty.cend());
    ASSERT_EQ(empty.cend() - 0, empty.cbegin());

    Deque<int> d(1000, 3);
    ASSERT_EQ(d.end() - d.begin(), d.Size());
    ASSERT_EQ(d.begin() + d.Size(), d.end());
    ASSERT_EQ(d.Size() + d.begin(), d.end());
    ASSERT_EQ(d.end() - d.Size(), d.begin());
}

TEST(Deque, IteratorsComparison) {
    Deque<int> d(1000, 3);

    ASSERT_GT(d.end(), d.begin());
    ASSERT_GT(d.cend(), d.cbegin());
    ASSERT_GT(d.rend(), d.rbegin());
}

TEST(Deque, IteratorsAlgorithms) {
    Deque<int> d(1000, 3);

    std::iota(d.begin(), d.end(), 13);
    std::mt19937 g(31415);
    std::shuffle(d.begin(), d.end(), g);
    std::sort(d.rbegin(), d.rbegin() + 500);
    std::reverse(d.begin(), d.end());
    auto sorted_border = std::is_sorted_until(d.begin(), d.end());

    ASSERT_EQ(sorted_border - d.begin(), 500);
}

TEST(Deque, PushAndPop) {
    Deque<NotDefaultConstructible> d(10000, {1});
    auto start_size = d.Size();

    auto middle = &(*(d.begin() + start_size / 2));  // 5000
    auto& middle_element = *middle;
    auto begin = &(*d.begin());
    auto end = &(*d.rbegin());

    auto middle2 = &(*((d.begin() + start_size / 2) + 2000));  // 7000

    // remove 400 elements
    for (size_t i = 0; i < 400; ++i) {
        d.PopBack();
    }

    // begin and middle pointers are still valid
    ASSERT_EQ(begin->data, 1);
    ASSERT_EQ(middle->data, 1);
    ASSERT_EQ(middle_element.data, 1);
    ASSERT_EQ(middle2->data, 1);

    end = &*d.rbegin();

    // 800 elemets removed in total
    for (size_t i = 0; i < 400; ++i) {
        d.PopFront();
    }

    // and and middle iterators are still valid
    ASSERT_EQ(end->data, 1);
    ASSERT_EQ(middle->data, 1);
    ASSERT_EQ(middle_element.data, 1);
    ASSERT_EQ(middle2->data, 1);

    // removed 9980 items in total
    for (size_t i = 0; i < 4590; ++i) {
        d.PopFront();
        d.PopBack();
    }

    ASSERT_EQ(d.Size(), 20);
    ASSERT_EQ(middle_element.data, 1);
    ASSERT_EQ(middle->data, 1);
    ASSERT_TRUE(std::all_of(d.begin(), d.end(), [](const auto& item) { return item.data == 1; }));

    begin = &*d.begin();
    end = &*d.rbegin();

    for (size_t i = 0; i < 5500; ++i) {
        d.PushBack({2});
        d.PushFront({2});
    }

    ASSERT_EQ((*begin).data, 1);
    ASSERT_EQ((*end).data, 1);
    ASSERT_EQ(d.begin()->data, 2);
    ASSERT_EQ(d.Size(), 5500 * 2 + 20);
    ASSERT_EQ(std::count(d.begin(), d.end(), NotDefaultConstructible{1}), 20);
    ASSERT_EQ(std::count(d.begin(), d.end(), NotDefaultConstructible{2}), 11000);
}

TEST(Deque, InsertAndErase) {
    Deque<NotDefaultConstructible> d(10000, {1});
    auto start_size = d.Size();

    d.Insert(d.begin() + start_size / 2, NotDefaultConstructible{2});
    ASSERT_EQ(d.Size(), start_size + 1);
    d.Erase(d.begin() + start_size / 2 - 1);
    ASSERT_EQ(d.Size(), start_size);

    ASSERT_EQ(size_t(std::count(d.begin(), d.end(), NotDefaultConstructible{1})), start_size - 1);
    ASSERT_EQ(std::count(d.begin(), d.end(), NotDefaultConstructible{2}), 1);

    Deque<NotDefaultConstructible> copy;
    for (const auto& item : d) {
        copy.Insert(copy.end(), item);
    }

    ASSERT_EQ(d.Size(), copy.Size());
    ASSERT_TRUE(std::equal(d.begin(), d.end(), copy.begin()));
}

TEST(Deque, Exceptions) {
    ASSERT_THROW(Deque<Counted<17>> d(100), CountedException);
    ASSERT_EQ(Counted<17>::counter, 0);

    ASSERT_THROW(Deque<Explosive>(100), ExplosiveException);
    // Destructor should not be called for an object with no finished constructor.
    // The only destructor called - safe explosive with the safeguard
    ASSERT_EQ(Explosive::exploded, false);

    Deque<Explosive> d;
    // No objects should have been created
    ASSERT_EQ(Explosive::exploded, false);

    auto safe = Explosive(Explosive::Safeguard{});
    ASSERT_THROW(d.PushBack(safe), ExplosiveException);
    ASSERT_EQ(Explosive::exploded, false);
    ASSERT_EQ(d.Size(), 0);

    ASSERT_THROW(d.PushFront(safe), ExplosiveException);
    ASSERT_EQ(Explosive::exploded, false);
    ASSERT_EQ(d.Size(), 0);

    ASSERT_THROW(d.Insert(d.begin(), safe), ExplosiveException);
    ASSERT_EQ(Explosive::exploded, false);
    ASSERT_EQ(d.Size(), 0);
}

}  // namespace testsuite2

namespace testsuite3 {
// By DarkCodeForce

struct Spectator {
    static ssize_t balance;

    Spectator() {
        SanityCheck();
        ++balance;
    }

    Spectator(const Spectator&) {
        SanityCheck();
        ++balance;
    }

    Spectator(Spectator&&) {
        SanityCheck();
        ++balance;
    }

    ~Spectator() {
        --balance;
        SanityCheck();
    }

    Spectator& operator=(const Spectator&) {
        return *this;
    }

    Spectator& operator=(Spectator&&) {
        return *this;
    };

    void SanityCheck() {
        ASSERT_GE(balance, 0) << "More destructor calls than constructor calls";
    }
};

ssize_t Spectator::balance = 0;

struct ExplosiveSpectatorError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct ExplosiveSpectator : public Spectator {
    mutable int delay;

public:
    ExplosiveSpectator() : ExplosiveSpectator(-1) {
    }

    ExplosiveSpectator(int delay) : Spectator(), delay(delay) {
    }

    ExplosiveSpectator(const ExplosiveSpectator& other)
        : Spectator(other), delay(other.delay <= 0 ? other.delay : other.delay - 1) {
        other.delay = delay;
        if (!delay) {
            throw ExplosiveSpectatorError("KABOOM!");
        }
    }

    ExplosiveSpectator(ExplosiveSpectator&& other)
        : Spectator(other), delay(other.delay <= 0 ? other.delay : other.delay - 1) {
        other.delay = delay;
        if (!delay) {
            throw ExplosiveSpectatorError("KABOOM!");
        }
    }

    ~ExplosiveSpectator() = default;

public:
    ExplosiveSpectator& operator=(const ExplosiveSpectator& other) {
        ExplosiveSpectator tmp(other);
        delay = tmp.delay;
        return *this;
    }

    ExplosiveSpectator& operator=(ExplosiveSpectator&& other) {
        ExplosiveSpectator tmp(other);
        delay = tmp.delay;
        return *this;
    }
};

struct BucketTest : ::testing::TestWithParam<size_t> {};

TEST_P(BucketTest, Stability) {
    size_t size = GetParam();
    {
        Deque<Spectator> d(size);
        ASSERT_EQ(Spectator::balance, size) << "Constructors are not called in required quantity";
        d.PopFront();
        ASSERT_EQ(Spectator::balance, size - 1) << "PopFront does not destroy object";
        d.PopBack();
        ASSERT_EQ(Spectator::balance, size - 2) << "PopBack does not destroy object";
        d.PushFront(Spectator());
        ASSERT_EQ(Spectator::balance, size - 1) << "PushFront does not construct object";
        d.PushBack(Spectator());
        ASSERT_EQ(Spectator::balance, size) << "PushBack does not construct object";
    }
    ASSERT_EQ(Spectator::balance, 0) << "Destructor does not properly destroy objects";
}

TEST_P(BucketTest, ExceptionSafety) {
    size_t size = GetParam();
    {
        Deque<ExplosiveSpectator> d(size);

        ASSERT_THROW(d.PushFront(ExplosiveSpectator(0)), ExplosiveSpectatorError);
        ASSERT_EQ(d.Size(), size);
        ASSERT_EQ(Spectator::balance, size);

        ASSERT_THROW(d.PushBack(ExplosiveSpectator(0)), ExplosiveSpectatorError);
        ASSERT_EQ(d.Size(), size);
        ASSERT_EQ(Spectator::balance, size);

        {
            Deque<ExplosiveSpectator> copy = d;
            copy.PushBack(ExplosiveSpectator());
            copy.rbegin()->delay = 0;
            ASSERT_EQ(copy.Size(), size + 1);
            ASSERT_EQ(Spectator::balance, size * 2 + 1);

            ASSERT_THROW(d = copy, ExplosiveSpectatorError);
        }
        ASSERT_EQ(d.Size(), size);
        ASSERT_EQ(Spectator::balance, size);

        d.rbegin()->delay = 0;
        ASSERT_THROW(Deque<ExplosiveSpectator> copy = d, ExplosiveSpectatorError);
        ASSERT_EQ(Spectator::balance, size);
    }

    ASSERT_EQ(Spectator::balance, 0);

    ASSERT_THROW(Deque<ExplosiveSpectator>(size, ExplosiveSpectator(size - 1)),
                 ExplosiveSpectatorError);
    ASSERT_EQ(Spectator::balance, 0);
}

INSTANTIATE_TEST_SUITE_P(Deque, BucketTest, ::testing::Range<size_t>(2, 1026));

}  // namespace testsuite3

size_t allocations_count = 0;
size_t max_allocation_size = 0;
bool need_throw_bad_alloc = false;

void* operator new(std::size_t count) {
    if (need_throw_bad_alloc) {
        throw std::bad_alloc{};
    }
    if (void* ptr = std::malloc(std::max(count, 1ul))) {
        ++allocations_count;
        max_allocation_size = std::max(max_allocation_size, count);
        return ptr;
    }
    throw std::bad_alloc{};
}

void* operator new[](std::size_t count) {
    return operator new(count);
}

void* operator new(std::size_t count, std::align_val_t /*al*/) {
    return operator new(count);
}

void* operator new[](std::size_t count, std::align_val_t /*al*/) {
    return operator new(count);
}

void* operator new(std::size_t count, const std::nothrow_t& /*tag*/) noexcept {
    try {
        return operator new(count);
    } catch (...) {
        return nullptr;
    }
}

void* operator new[](std::size_t count, const std::nothrow_t& /*tag*/) noexcept {
    try {
        return operator new(count);
    } catch (...) {
        return nullptr;
    }
}

void* operator new(std::size_t count, std::align_val_t al, const std::nothrow_t& /*tag*/) noexcept {
    try {
        return operator new(count, al);
    } catch (...) {
        return nullptr;
    }
}

void* operator new[](std::size_t count, std::align_val_t al,
                     const std::nothrow_t& /*tag*/) noexcept {
    try {
        return operator new(count, al);
    } catch (...) {
        return nullptr;
    }
}

void operator delete(void* ptr) noexcept {
    std::free(ptr);
}

void operator delete[](void* ptr) noexcept {
    std::free(ptr);
}

void operator delete(void* ptr, std::size_t /*count*/) noexcept {
    std::free(ptr);
}

void operator delete[](void* ptr, std::size_t /*count*/) noexcept {
    std::free(ptr);
}

void operator delete(void* ptr, std::align_val_t /*al*/) noexcept {
    std::free(ptr);
}

void operator delete[](void* ptr, std::align_val_t /*al*/) noexcept {
    std::free(ptr);
}

void operator delete(void* ptr, std::size_t /*count*/, std::align_val_t /*al*/) noexcept {
    std::free(ptr);
}

void operator delete[](void* ptr, std::size_t /*count*/, std::align_val_t /*al*/) noexcept {
    std::free(ptr);
}

void operator delete(void* ptr, const std::nothrow_t& /*tag*/) noexcept {
    std::free(ptr);
}

void operator delete[](void* ptr, const std::nothrow_t& /*tag*/) noexcept {
    std::free(ptr);
}

void operator delete(void* ptr, std::align_val_t /*al*/, const std::nothrow_t& /*tag*/) noexcept {
    std::free(ptr);
}

void operator delete[](void* ptr, std::align_val_t /*al*/, const std::nothrow_t& /*tag*/) noexcept {
    std::free(ptr);
}

namespace testsuite4 {
// By TrofiMichael

TEST(Deque, DefaultConstructorNotAllocate) {
    allocations_count = 0;

    Deque<int> deque;
    ASSERT_EQ(allocations_count, 0)
        << "Default constructor should not allocate memory (unlike in the STL's implementation)";
}

TEST(Deque, LazyAllocationBuckets) {
    constexpr size_t kElements = 10000;
    constexpr size_t kMinBucketSize = 16;
    constexpr size_t kMaxBucketCount = (kElements + kMinBucketSize - 1) / kMinBucketSize;
    constexpr size_t kMaxAllocCount = kMaxBucketCount * 3 / 2 + 2;

    allocations_count = 0;

    Deque<int> deque;
    for (size_t i = 0; i < kElements; ++i) {
        size_t cur_alloc = allocations_count;
        if (i % 2 == 0) {
            deque.PushBack(i);
        } else {
            deque.PushFront(i);
        }
        ASSERT_LE(allocations_count, cur_alloc + 2)
            << "Too many allocations, buckets should be allocated lazily (or bucket size is lower "
               "that 16)";
    }

    ASSERT_LE(allocations_count, kMaxAllocCount) << "Too many allocations";
}

TEST(Deque, BucketSizeUpperBound) {
    constexpr size_t kMaxBucketSize = 1024;

    max_allocation_size = 0;

    Deque<int> deque;
    deque.PushBack(1);

    ASSERT_LE(max_allocation_size, kMaxBucketSize * sizeof(int))
        << "Bucket size is too big, set it to no more than 1024";
}

TEST(Deque, EmptyDequeIteratorsCheck) {
    Deque<int> deque;
    ASSERT_EQ(deque.begin(), deque.end());
}

TEST(Deque, ReferenceNotInvalidatedAfterOperations) {
    constexpr int kSize = 1024;

    Deque<int> deque(kSize);
    std::iota(deque.begin(), deque.end(), 0);

    std::vector<std::reference_wrapper<int>> refs;
    std::ranges::copy(deque, std::back_inserter(refs));

    for (int i = 0; i < kSize; ++i) {
        ASSERT_EQ(refs[i], i);
    }

    for (int i = 0; i < kSize * 4; ++i) {
        if (i % 4 == 0) {
            deque.PushBack(i);
        } else if (i % 4 == 1) {
            deque.PushFront(i);
        } else if (i % 4 == 2) {
            deque.Insert(deque.begin(), i);
        } else {
            deque.Insert(deque.end(), i);
        }

        for (int j = 0; j < kSize; ++j) {
            ASSERT_EQ(refs[j], j);
        }
    }
}

TEST(Deque, RandomAccessIterator) {
    static_assert(std::random_access_iterator<Deque<int>::iterator>);
    static_assert(std::random_access_iterator<Deque<int>::const_iterator>);
    static_assert(std::random_access_iterator<Deque<int>::reverse_iterator>);
    static_assert(std::random_access_iterator<Deque<int>::const_reverse_iterator>);
}

struct ThrowNewAfterConstruct {
    inline static int counter = 0;

    ThrowNewAfterConstruct() {
        ++counter;
    }

    ThrowNewAfterConstruct(const ThrowNewAfterConstruct&) {
        ++counter;
        need_throw_bad_alloc = true;
    }

    ThrowNewAfterConstruct(ThrowNewAfterConstruct&&) noexcept {
        ++counter;
        need_throw_bad_alloc = true;
    }

    ~ThrowNewAfterConstruct() {
        --counter;
    }
};

TEST(Deque, StrongGuarantee) {
    // check that the strong guarantee is observed when throwing exception in new after creating
    // object in PushBack/PushFront
    constexpr size_t kMaxBucketSize = 1024;

    for (int type = 0; type < 2; ++type) {
        Deque<ThrowNewAfterConstruct> deque;
        for (size_t i = 0; i < kMaxBucketSize * 4; ++i) {
            size_t cur_count = ThrowNewAfterConstruct::counter;
            try {
                if (type == 0) {
                    deque.PushBack(ThrowNewAfterConstruct());
                } else {
                    deque.PushFront(ThrowNewAfterConstruct());
                }
            } catch (...) {
                ASSERT_EQ(cur_count, ThrowNewAfterConstruct::counter);
            }
            need_throw_bad_alloc = false;
        }
    }
}

class ThrowIfNeed {
public:
    ThrowIfNeed(bool need_throw, int value) noexcept : need_throw_(need_throw), value_(value) {
    }

    ThrowIfNeed(const ThrowIfNeed& other) : need_throw_(other.need_throw_), value_(other.value_) {
        if (need_throw_) {
            throw std::invalid_argument("ThrowIfNeed");
        }
    }

    friend bool operator==(const ThrowIfNeed& lhs, const ThrowIfNeed& rhs) noexcept {
        return lhs.value_ == rhs.value_;
    }

public:
    bool need_throw_ = false;
    int value_ = 0;
};

struct BucketTestIterators : ::testing::TestWithParam<int> {};

TEST_P(BucketTestIterators, IteratorInvalidationStrongGuarantee) {
    // check that iterators are not invalidated in PushBack/PushFront after an exception is thrown
    // from the object constructor

    int size = GetParam();
    using Iter = Deque<ThrowIfNeed>::iterator;

    for (int type = 0; type < 2; ++type) {
        Deque<ThrowIfNeed> deque;
        for (int i = 0; i < size; ++i) {
            ThrowIfNeed safe(false, i);
            deque.PushBack(safe);
        }

        std::vector<Iter> iters(size);
        std::iota(iters.begin(), iters.end(), deque.begin());
        for (int i = 0; i < size; ++i) {
            ASSERT_EQ(*(iters[i]), ThrowIfNeed(false, i));
        }

        try {
            ThrowIfNeed unsafe(true, 0);
            if (type == 0) {
                deque.PushBack(unsafe);
            } else {
                deque.PushFront(unsafe);
            }
            ASSERT_TRUE(false);  // exception must be thrown
        } catch (...) {
            auto iter = iters[0];
            for (int i = 0; i < size; ++i) {
                ASSERT_EQ(*(iters[i]), ThrowIfNeed(false, i));
                ASSERT_EQ(*iter, ThrowIfNeed(false, i));
                ++iter;
            }
        }
    }
}

INSTANTIATE_TEST_SUITE_P(Deque, BucketTestIterators, ::testing::Range<int>(2, 1026));

}  // namespace testsuite4
