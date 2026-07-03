#pragma once

#include <cstddef>
#include <memory>

#include <sys/resource.h>

constexpr size_t kSmallSize = 17;
constexpr size_t kMediumSize = 100;
constexpr size_t kBigSize = 10'000;
constexpr int kNontrivialInt = 42;

template <typename T>
struct IotaIterator {
    T current;

    const T& operator*() const {
        return current;
    }

    const T* operator->() const {
        return &current;
    }

    IotaIterator& operator++() {
        ++current;
        return *this;
    }

    IotaIterator operator++(int) {
        auto copy = current;
        operator++();
        return {copy};
    }

    auto operator<=>(const IotaIterator&) const = default;
};

template <typename T>
struct ReversedIotaIterator {
    T current;

    const T& operator*() const {
        return current;
    }

    const T* operator->() const {
        return &current;
    }

    ReversedIotaIterator& operator++() {
        --current;
        return *this;
    }

    ReversedIotaIterator operator++(int) {
        auto copy = current;
        operator++();
        return {copy};
    }

    auto operator<=>(const ReversedIotaIterator&) const = default;
};

struct AllocCounterContainer {
    static size_t alloc_calls;
    static size_t dealloc_calls;
};

template <typename T>
struct AllocatorCounterWrap {
    using value_type = T;

    template <typename U>
    AllocatorCounterWrap(const AllocatorCounterWrap<U>&) {
    }

    AllocatorCounterWrap() = default;

    void Reset() {
        AllocCounterContainer::alloc_calls = 0;
        AllocCounterContainer::dealloc_calls = 0;
    }

    T* allocate(size_t n) {
        ++AllocCounterContainer::alloc_calls;
        return std::allocator<T>().allocate(n);
    }

    void deallocate(T* ptr, size_t n) {
        ++AllocCounterContainer::dealloc_calls;
        std::allocator<T>().deallocate(ptr, n);
    }
};

size_t AllocCounterContainer::alloc_calls = 0;
size_t AllocCounterContainer::dealloc_calls = 0;

template <typename T, bool PropagateOnConstruct, bool PropagateOnAssign>
struct WhimsicalAllocator : public std::allocator<T> {
    std::shared_ptr<int> number;

    auto select_on_container_copy_construction() const {
        return PropagateOnConstruct
                   ? WhimsicalAllocator<T, PropagateOnConstruct, PropagateOnAssign>()
                   : *this;
    }

    using propagate_on_container_copy_assignment =
        std::conditional_t<PropagateOnAssign, std::true_type, std::false_type>;

    template <typename U>
    struct rebind {
        using other = WhimsicalAllocator<U, PropagateOnConstruct, PropagateOnAssign>;
    };

    WhimsicalAllocator() : number(std::make_shared<int>(counter)) {
        ++counter;
    }

    template <typename U>
    WhimsicalAllocator(
        const WhimsicalAllocator<U, PropagateOnConstruct, PropagateOnAssign>& another)
        : number(another.number) {
    }

    template <typename U>
    auto& operator=(const WhimsicalAllocator<U, PropagateOnConstruct, PropagateOnAssign>& another) {
        number = another.number;
        return *this;
    }

    template <typename U>
    bool operator==(
        const WhimsicalAllocator<U, PropagateOnConstruct, PropagateOnAssign>& another) const {
        return std::is_same_v<decltype(*this), decltype(another)> && *number == *another.number;
    }

    static size_t counter;
};

template <typename T, bool PropagateOnConstruct, bool PropagateOnAssign>
size_t WhimsicalAllocator<T, PropagateOnConstruct, PropagateOnAssign>::counter = 0;

struct CountedException : public std::exception {};

template <int WhenThrow>
struct Counted {
    inline static int counter = 0;

    Counted() {
        ++counter;
        if (counter == WhenThrow) {
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
