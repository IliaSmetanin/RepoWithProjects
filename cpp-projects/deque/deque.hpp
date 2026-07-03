#pragma once

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <vector>

template <typename T>
class Deque {
public:
    template <bool CONST>
    class Iterator {
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = std::conditional_t<CONST, const T*, T*>;
        using reference = std::conditional_t<CONST, const T&, T&>;
        using iterator_category = std::random_access_iterator_tag;

        Iterator() noexcept;

        Iterator(pointer* bucket_ptr, pointer elem_ptr, pointer* last_bucket) noexcept;

        Iterator(const Iterator& right) noexcept;

        operator Iterator<true>() const noexcept;

        Iterator& operator=(const Iterator& right) noexcept;

        Iterator& operator+=(ptrdiff_t right);

        Iterator& operator-=(ptrdiff_t right);

        Iterator& operator++() noexcept;

        Iterator operator++(int) noexcept;

        Iterator& operator--() noexcept;

        Iterator operator--(int) noexcept;

        reference operator*() const noexcept;

        pointer operator->() const noexcept;

        reference operator[](difference_type index) noexcept;

        reference operator[](difference_type index) const noexcept;

        pointer* GetBucket() const noexcept;

        pointer GetElement() const noexcept;

        friend Iterator operator+(const Iterator& iter, ptrdiff_t right) {
            Iterator<CONST> temp(iter);
            temp += right;
            return temp;
        }

        friend Iterator operator+(ptrdiff_t left, const Iterator& iter) {
            return iter + left;
        }

        friend Iterator operator-(const Iterator& iter, ptrdiff_t right) {
            Iterator<CONST> temp(iter);
            temp -= right;
            return temp;
        }

        friend ptrdiff_t operator-(const Iterator& left, const Iterator& right) noexcept {
            if (!left.GetBucket() || !right.GetBucket() || !left.GetElement() ||
                !right.GetElement()) {
                return 0;
            }

            ptrdiff_t bucket_dif = left.GetBucket() - right.GetBucket();
            ptrdiff_t left_elements = left.GetElement() - *left.GetBucket();
            ptrdiff_t right_elements = right.GetElement() - *right.GetBucket();

            return kBucketSize * bucket_dif + (left_elements - right_elements);
        }

        friend bool operator==(const Iterator& left, const Iterator& right) noexcept {
            return (left - right) == 0;
        }

        friend std::strong_ordering operator<=>(const Iterator& left, const Iterator& right) {
            return (left - right) <=> 0;
        }

    private:
        pointer* bucket_ptr_;
        pointer elem_ptr_;
        pointer* last_bucket_;
    };

    using const_iterator = Iterator<true>;
    using iterator = Iterator<false>;
    using const_reverse_iterator = std::reverse_iterator<Iterator<true>>;
    using reverse_iterator = std::reverse_iterator<Iterator<false>>;

    Deque();

    Deque(const Deque& right);

    Deque(int32_t size);

    Deque(int32_t size, const T& value);

    ~Deque();

    Deque& operator=(const Deque& right);

    void Swap(Deque& right) noexcept;

    T& operator[](size_t index);

    const T& operator[](size_t index) const;

    T& At(size_t index);

    const T& At(size_t index) const;

    void PushBack(const T& value);

    void PushFront(const T& value);

    void PopBack();

    void PopFront();

    iterator begin() noexcept;

    const_iterator begin() const noexcept;

    const_iterator cbegin() const noexcept;

    iterator end() noexcept;

    const_iterator end() const noexcept;

    const_iterator cend() const noexcept;

    reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }

    const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(begin());
    }

    void Insert(const_iterator it, const T& value);

    void Erase(const_iterator it);

    size_t BucketsCount() const noexcept {
        return buckets_ptrs_.size();
    }

    size_t Size() const noexcept {
        return size_;
    }

private:
    static const ptrdiff_t kBucketSize = 64;

    void BackOffConstruction(size_t index) noexcept;

    void SetMembersToZero() noexcept;

    T* CreateBucket();

    void RemoveBucket(T* bucket_ptr);

    void AddBucketBack();

    void AddBucketFront();

    void AddBuckets(size_t size);

    size_t size_;
    size_t first_pos_;
    size_t last_pos_;
    size_t first_bucket_;
    size_t last_bucket_;
    std::vector<T*> buckets_ptrs_;
};

#include "src/Iterator.tpp"
#include "src/deque.tpp"