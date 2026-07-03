#include <cstddef>

template <typename T, bool CONST>
using DequeIterator = typename Deque<T>::template Iterator<CONST>;

template <typename T>
template <bool CONST>
Deque<T>::Iterator<CONST>::Iterator() noexcept
    : bucket_ptr_(nullptr), elem_ptr_(nullptr), last_bucket_(nullptr) {
}

template <typename T>
template <bool CONST>
Deque<T>::Iterator<CONST>::Iterator(pointer* bucket_ptr, pointer elem_ptr, pointer* last_bucket) noexcept
    : bucket_ptr_(bucket_ptr), elem_ptr_(elem_ptr), last_bucket_(last_bucket) {
}

template <typename T>
template <bool CONST>
Deque<T>::Iterator<CONST>::Iterator(const Deque<T>::Iterator<CONST>& right) noexcept
    : bucket_ptr_(right.bucket_ptr_), elem_ptr_(right.elem_ptr_), last_bucket_(right.last_bucket_) {
}

template <typename T>
template <bool CONST>
Deque<T>::Iterator<CONST>::operator Deque<T>::Iterator<true>() const noexcept {
    return Deque<T>::Iterator<true>(const_cast<const T**>(bucket_ptr_), const_cast<const T*>(elem_ptr_),
                             const_cast<const T**>(last_bucket_));
}

template <typename T>
template <bool CONST>
Deque<T>::Iterator<CONST>& Deque<T>::Iterator<CONST>::operator=(const Deque<T>::Iterator<CONST>& right) noexcept {
    bucket_ptr_ = right.bucket_ptr_;
    elem_ptr_ = right.elem_ptr_;
    last_bucket_ = right.last_bucket_;
    return *this;
}

template <typename T>
template <bool CONST>
Deque<T>::Iterator<CONST>& Deque<T>::Iterator<CONST>::operator+=(ptrdiff_t right) {
    if (!bucket_ptr_ || !elem_ptr_) {
        return *this;
    }

    if (right < 0) {
        *this -= -right;
        return *this;
    }

    size_t buckets = right / kBucketSize;
    size_t elements = right % kBucketSize;
    size_t existing_index = elem_ptr_ - *bucket_ptr_;
    if (existing_index + elements >= kBucketSize) {
        ++bucket_ptr_;
    }

    bucket_ptr_ += buckets;
    elem_ptr_ = *bucket_ptr_ + (existing_index + elements) % kBucketSize;
    return *this;
}

template <typename T>
template <bool CONST>
Deque<T>::Iterator<CONST>& Deque<T>::Iterator<CONST>::operator-=(ptrdiff_t right) {
    if (!bucket_ptr_ || !elem_ptr_) {
        return *this;
    }

    if (right < 0) {
        *this += -right;
        return *this;
    }

    // special case for end()
    ptrdiff_t existing_index = elem_ptr_ - *bucket_ptr_;
    if (existing_index == kBucketSize && right > 0) {
        --right;
        --existing_index;
    }

    ptrdiff_t buckets = right / kBucketSize;
    ptrdiff_t elements = right % kBucketSize;
    ptrdiff_t offset = 0;
    if (elements > existing_index) {
        --bucket_ptr_;
        offset += kBucketSize;
    }

    ptrdiff_t pos = offset + existing_index - elements;
    bucket_ptr_ -= buckets;
    elem_ptr_ = *bucket_ptr_ + pos;

    return *this;
}

template <typename T>
template <bool CONST>
Deque<T>::Iterator<CONST>& Deque<T>::Iterator<CONST>::operator++() noexcept {
    ++elem_ptr_;
    if (elem_ptr_ == *bucket_ptr_ + kBucketSize) {
        if (bucket_ptr_ == last_bucket_) {
            return *this;
        }
        ++bucket_ptr_;
        elem_ptr_ = *bucket_ptr_;
    }

    return *this;
}

template <typename T>
template <bool CONST>
Deque<T>::Iterator<CONST> Deque<T>::Iterator<CONST>::operator++(int) noexcept {
    Deque<T>::Iterator<CONST> temp = *this;
    ++(*this);

    return temp;
}

template <typename T>
template <bool CONST>
Deque<T>::Iterator<CONST>& Deque<T>::Iterator<CONST>::operator--() noexcept {
    if (bucket_ptr_ > last_bucket_) {
        --bucket_ptr_;
        elem_ptr_ = *bucket_ptr_ + kBucketSize - 1;
    }

    if (elem_ptr_ == *bucket_ptr_) {
        --bucket_ptr_;
        elem_ptr_ = *bucket_ptr_ + kBucketSize - 1;
    } else {
        --elem_ptr_;
    }

    return *this;
}

template <typename T>
template <bool CONST>
Deque<T>::Iterator<CONST> Deque<T>::Iterator<CONST>::operator--(int) noexcept {
    Deque<T>::Iterator<CONST> temp = *this;
    --(*this);
    return temp;
}

template <typename T>
template <bool CONST>
Deque<T>::Iterator<CONST>::reference Deque<T>::Iterator<CONST>::operator*() const noexcept {
    return *elem_ptr_;
}

template <typename T>
template <bool CONST>
Deque<T>::Iterator<CONST>::pointer Deque<T>::Iterator<CONST>::operator->() const noexcept {
    return elem_ptr_;
}

template <typename T>
template <bool CONST>
Deque<T>::Iterator<CONST>::reference Deque<T>::Iterator<CONST>::operator[](difference_type index) noexcept {
    return *(*this + index);
}

template <typename T>
template <bool CONST>
Deque<T>::Iterator<CONST>::reference Deque<T>::Iterator<CONST>::operator[](difference_type index) const noexcept {
    return *(*this + index);
}

template <typename T>
template <bool CONST>
Deque<T>::Iterator<CONST>::pointer* Deque<T>::Iterator<CONST>::GetBucket() const noexcept {
    return bucket_ptr_;
}

template <typename T>
template <bool CONST>
Deque<T>::Iterator<CONST>::pointer Deque<T>::Iterator<CONST>::GetElement() const noexcept {
    return elem_ptr_;
}
