#pragma once

#include <cstddef>
#include <cstdint>
#include <new>
#include <type_traits>
#include <vector>

template <size_t N>
class PoolStorage {
public:
    PoolStorage() {
        free_areas_.push_back({0, N - 1});
    }

    PoolStorage(const PoolStorage& pool_storage) = delete;

    const std::byte* GetStorage() const noexcept {
        return storage_;
    }

    std::byte* AllocateArea(size_t bytes, std::align_val_t alignment) {
        for (size_t i = 0; i < free_areas_.size(); ++i) {
            size_t start = free_areas_[i].first;
            size_t end = free_areas_[i].second;
            size_t window = end - start + 1;

            size_t offset = GetOffset(storage_ + start, alignment);
            if (bytes + offset > window) {
                continue;
            }

            size_t aligned_start = start + offset;
            size_t new_start = aligned_start + bytes;

            if (i != free_areas_.size() - 1) {
                std::swap(free_areas_[i], free_areas_.back());
            }
            free_areas_.pop_back();

            if (start < aligned_start) {
                free_areas_.push_back({start, aligned_start - 1});
            }

            if (new_start <= end) {
                free_areas_.push_back({new_start, end});
            }

            return storage_ + aligned_start;
        }

        throw std::bad_alloc();
    }

    void FreeArea(std::byte* pool_ptr, size_t bytes) {
        size_t start = pool_ptr - storage_;
        size_t end = start + bytes - 1;

        size_t prev = UINT64_MAX;
        size_t next = UINT64_MAX;

        size_t free_areas_size = free_areas_.size();
        for (size_t i = 0; i < free_areas_size; ++i) {
            if (start == free_areas_[i].second + 1) {
                prev = i;
            }

            if (free_areas_[i].first == end + 1) {
                next = i;
            }
        }

        if (prev != UINT64_MAX && next != UINT64_MAX) {
            free_areas_[prev].second = free_areas_[next].second;
            std::swap(free_areas_.back(), free_areas_[next]);

            return;
        }

        if (prev != UINT64_MAX) {
            free_areas_[prev].second = end;
            return;
        }

        if (next != UINT64_MAX) {
            free_areas_[next].first = start;
            return;
        }

        free_areas_.push_back({start, end});
    }

private:
    static size_t GetOffset(std::byte* ptr, std::align_val_t alignment) {
        size_t align_val = static_cast<size_t>(alignment);
        size_t offset = reinterpret_cast<uintptr_t>(ptr) % align_val;
        return (offset == 0) ? 0 : align_val - offset;
    }

    alignas(std::max_align_t) std::byte storage_[N];
    std::vector<std::pair<size_t, size_t>> free_areas_;
};

template <typename T, size_t N>
class PoolAllocator {
public:
    template <typename U>
    struct rebind {
        using other = PoolAllocator<U, N>;
    };

    using pointer = T*;

    using const_pointer = const T*;

    // using void_pointer = void*;

    // using const_void_pointer = const void*;

    using reference = T&;

    using const_reference = const T&;

    using value_type = T;

    using size_type = size_t;

    using difference_type = ptrdiff_t;

    using propagate_on_container_copy_assignment = std::true_type;

    using propagate_on_container_swap = std::true_type;

    PoolAllocator() noexcept = delete;

    PoolAllocator(PoolStorage<N>& storage) noexcept : storage_ptr_(&storage) {
    }

    PoolAllocator(const PoolAllocator& right) noexcept = default;

    template <typename U>
    PoolAllocator(const PoolAllocator<U, N>& right) noexcept : storage_ptr_(right.storage_ptr_) {
    }

    PoolAllocator select_on_container_copy_construction() const noexcept {
        return *this;
    }

    ~PoolAllocator() = default;

    PoolAllocator& operator=(const PoolAllocator& right) noexcept = default;

    pointer allocate(size_t n) {
        return reinterpret_cast<pointer>(
            storage_ptr_->AllocateArea(n * sizeof(T), static_cast<std::align_val_t>(alignof(T))));
    }

    void deallocate(pointer ptr, size_t n) {
        storage_ptr_->FreeArea(reinterpret_cast<std::byte*>(ptr), n * sizeof(T));
    }

    void construct(pointer ptr, const_reference value) {
        ::new (static_cast<void*>(ptr)) value_type(value);
    }

    template <typename... Args>
    void construct(pointer ptr, const Args&... args) {
        ::new (static_cast<void*>(ptr)) value_type(args...);
    }

    void destroy(pointer ptr) noexcept {
        ptr->~T();
    }

    const PoolStorage<N>* GetStorage() const noexcept {
        return storage_ptr_;
    }

private:
    // gcc complaining on not full class declaration
    // template <typename U>
    // friend class PoolAllocator<U, N>;
    template <typename U, size_t M>
    friend class PoolAllocator;

    PoolStorage<N>* storage_ptr_;
};

template <typename T, size_t N>
bool operator==(const PoolAllocator<T, N>& left, const PoolAllocator<T, N>& right) {
    return left.GetStorage() == right.GetStorage();
}