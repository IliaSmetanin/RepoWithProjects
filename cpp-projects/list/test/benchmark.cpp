#include "list.hpp"
#include "pool_allocator.hpp"
#include "stack_allocation.hpp"

#include <chrono>
#include <list>

#include <gtest/gtest.h>

template <typename T, typename Alloc>
struct ListAdapter : std::list<T, Alloc> {
    using Base = std::list<T, Alloc>;

    ListAdapter() = default;

    ListAdapter(Alloc alloc) : Base(alloc) {
    }

    void PushFront(const T& value) {
        Base::push_front(value);
    }

    void PopFront() {
        Base::pop_front();
    }

    void PushBack(const T& value) {
        Base::push_back(value);
    }

    void PopBack() {
        Base::pop_back();
    }

    Base::iterator Insert(Base::const_iterator it, const T& value) {
        return Base::insert(it, value);
    }

    Base::iterator Erase(Base::const_iterator it) {
        return Base::erase(it);
    }
};

template <class List>
std::optional<int> ListPerformanceTest(List&& l) {
    using Clock = std::chrono::high_resolution_clock;

    std::ostringstream oss;

    auto start = Clock::now();

    for (int i = 0; i < 1'000'000; ++i) {
        l.PushBack(i);
    }
    auto it = l.begin();
    for (int i = 0; i < 1'000'000; ++i) {
        l.PushFront(i);
    }
    oss << *it;

    auto it2 = std::prev(it);
    for (int i = 0; i < 2'000'000; ++i) {
        l.Insert(it, i);
        if (i % 534'555 == 0) {
            oss << *it;
        }
    }
    oss << *it;

    for (int i = 0; i < 1'500'000; ++i) {
        l.PopBack();
        if (i % 342'985 == 0) {
            oss << *l.rbegin();
        }
    }
    oss << *l.rbegin();

    for (int i = 0; i < 1'000'000; ++i) {
        l.Erase(it2++);
        if (i % 432'098 == 0) {
            oss << *it2;
        }
    }
    oss << *it2;

    for (int i = 0; i < 1'000'000; ++i) {
        l.PopFront();
    }
    oss << *l.begin();

    for (int i = 0; i < 1'000'000; ++i) {
        l.PushBack(i);
    }
    oss << *l.rbegin();

    std::string_view expected =
        "00000099999865701331402819710431628058149999904320988641969999991000000999999";
    if (oss.view() != expected) {
        ADD_FAILURE() << oss.view() << " is not equal to " << expected;
        return std::nullopt;
    }

    auto finish = Clock::now();
    return duration_cast<std::chrono::milliseconds>(finish - start).count();
}

template <template <typename, typename> class Container>
void TestPerformance() {
    std::ostringstream oss_first;
    std::ostringstream oss_second;

    {
        PoolStorage<kStorageSize> storage;
        PoolAllocator<int, kStorageSize> alloc(storage);

        if (!ListPerformanceTest(Container<int, std::allocator<int>>())) {
            return;
        }

        if (!ListPerformanceTest(Container<int, PoolAllocator<int, kStorageSize>>(alloc))) {
            return;
        }
    }

    double mean_stl_alloc = 0.0;
    double mean_stack_alloc = 0.0;

    int first = 0;
    int second = 0;

    constexpr int kAttempts = 10;

    for (int i = 0; i < kAttempts; ++i) {
        if (std::optional time = ListPerformanceTest(Container<int, std::allocator<int>>())) {
            first = *time;
        } else {
            return;
        }
        mean_stl_alloc += first;
        oss_first << first << " ";

        PoolStorage<kStorageSize> storage;
        PoolAllocator<int, kStorageSize> alloc(storage);
        if (std::optional time =
                ListPerformanceTest(Container<int, PoolAllocator<int, kStorageSize>>(alloc))) {
            second = *time;
        } else {
            return;
        }
        mean_stack_alloc += second;
        oss_second << second << " ";
    }

    mean_stl_alloc /= kAttempts;
    mean_stack_alloc /= kAttempts;

    std::cerr << "Results:\n"
              << "- std::allocator: " << oss_first.str() << " ms | Mean: " << mean_stl_alloc
              << " ms\n"
              << "- PoolAllocator: " << oss_second.str() << " ms | Mean: " << mean_stack_alloc
              << " ms" << std::endl;

    ASSERT_GT(mean_stl_alloc * 0.9, mean_stack_alloc);
}

TEST(Benchmark, Stl) {
    TestPerformance<ListAdapter>();
}

TEST(Benchmark, Yours) {
    TestPerformance<List>();
}