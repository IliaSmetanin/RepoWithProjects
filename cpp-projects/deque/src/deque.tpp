#include <cstddef>
#include <stdexcept>
#include <vector>

template <typename T>
Deque<T>::Deque() : size_(0), first_pos_(0), last_pos_(0), first_bucket_(0), last_bucket_(0) {
}

template <typename T>
Deque<T>::Deque(const Deque<T>& right)
    : size_(right.size_),
      first_pos_(0),
      last_pos_((right.size_ > 0) ? (right.size_ - 1) % kBucketSize : 0),
      first_bucket_(0),
      last_bucket_((right.size_ - 1) / kBucketSize) {
    AddBuckets(right.size_);

    size_t i = 0;
    try {
        for (; i < size_; ++i) {
            new (&(*this)[i]) T(right[i]);
        }
    } catch (...) {
        BackOffConstruction(i);
        throw;
    }
}

template <typename T>
Deque<T>::Deque(int32_t size)
    : size_(size),
      first_pos_(0),
      last_pos_((size > 0) ? (size - 1) % kBucketSize : 0),
      first_bucket_(0),
      last_bucket_((size - 1) / kBucketSize) {
    AddBuckets(size);

    size_t i = 0;
    try {
        for (; i < size_; ++i) {
            new (&(*this)[i]) T();
        }
    } catch (...) {
        BackOffConstruction(i);
        throw;
    }
}

template <typename T>
Deque<T>::Deque(int32_t size, const T& value)
    : size_(size),
      first_pos_(0),
      last_pos_((size > 0) ? (size - 1) % kBucketSize : 0),
      first_bucket_(0),
      last_bucket_((size - 1) / kBucketSize) {
    AddBuckets(size);

    size_t i = 0;
    try {
        for (; i < size_; ++i) {
            new (&(*this)[i]) T(value);
        }
    } catch (...) {
        BackOffConstruction(i);
        throw;
    }
}

template <typename T>
Deque<T>::~Deque() {
    for (size_t i = 0; i < size_; ++i) {
        (*this)[i].~T();
    }

    size_t buckets = BucketsCount();
    for (size_t i = 0; i < buckets; ++i) {
        RemoveBucket(buckets_ptrs_[i]);
    }
    buckets_ptrs_.clear();
}

template <typename T>
Deque<T>& Deque<T>::operator=(const Deque& right) {
    if (this == &right) {
        return *this;
    }

    Deque temp(right);
    Swap(temp);

    return *this;
}

template <typename T>
void Deque<T>::Swap(Deque& right) noexcept {
    std::swap(size_, right.size_);
    std::swap(first_pos_, right.first_pos_);
    std::swap(last_pos_, right.last_pos_);
    std::swap(first_bucket_, right.first_bucket_);
    std::swap(last_bucket_, right.last_bucket_);
    std::swap(buckets_ptrs_, right.buckets_ptrs_);
}

template <typename T>
T& Deque<T>::operator[](size_t index) {
    size_t common_index = first_pos_ + kBucketSize * first_bucket_ + index;
    return buckets_ptrs_[common_index / kBucketSize][common_index % kBucketSize];
}

template <typename T>
const T& Deque<T>::operator[](size_t index) const {
    size_t common_index = first_pos_ + kBucketSize * first_bucket_ + index;
    return buckets_ptrs_[common_index / kBucketSize][common_index % kBucketSize];
}

template <typename T>
T& Deque<T>::At(size_t index) {
    if (index >= size_) {
        throw std::out_of_range("Mistake in calling method At(): Out of range");
    }

    return (*this)[index];
}

template <typename T>
const T& Deque<T>::At(size_t index) const {
    if (index >= size_) {
        throw std::out_of_range("Mistake in calling method At(): Out of range");
    }

    return (*this)[index];
}

template <typename T>
void Deque<T>::PushBack(const T& value) {
    bool new_bucket = false;
    if (size_ == 0) {
        if (BucketsCount() == 0) {
            AddBucketBack();
            new_bucket = true;
        }

        first_pos_ = last_pos_ = kBucketSize / 2;
        first_bucket_ = last_bucket_ = 0;

        try {
            new (&(*this)[0]) T(value);
        } catch (...) {
            if (new_bucket) {
                RemoveBucket(buckets_ptrs_.back());
                buckets_ptrs_.pop_back();
            }
            SetMembersToZero();

            throw;
        }

        ++size_;

        return;
    }

    size_t old_last_pos = last_pos_;
    size_t old_last_bucket = last_bucket_;
    if (last_pos_ == kBucketSize - 1) {
        if (last_bucket_ == BucketsCount() - 1) {
            T* new_bucket = CreateBucket();
            try {
                new (new_bucket) T(value);
            } catch (...) {
                RemoveBucket(new_bucket);
                throw;
            }

            try {
                buckets_ptrs_.push_back(new_bucket);
            } catch (...) {
                new_bucket->~T();
                RemoveBucket(new_bucket);
                throw;
            }

            last_pos_ = 0;
            ++last_bucket_;
            ++size_;

            return;
        } else {
            last_pos_ = 0;
            ++last_bucket_;
        }
    } else {
        ++last_pos_;
    }

    try {
        new (&(*this)[size_]) T(value);
    } catch (...) {
        last_pos_ = old_last_pos;
        last_bucket_ = old_last_bucket;
        throw;
    }

    ++size_;
}

template <typename T>
void Deque<T>::PushFront(const T& value) {
    if (size_ == 0) {
        PushBack(value);
        return;
    }

    size_t old_first_bucket = first_bucket_;
    size_t old_first_pos = first_pos_;
    if (first_pos_ == 0) {
        if (first_bucket_ == 0) {
            T* new_bucket = CreateBucket();

            try {
                new (new_bucket + (kBucketSize - 1)) T(value);
            } catch (...) {
                RemoveBucket(new_bucket);
                throw;
            }

            try {
                buckets_ptrs_.insert(buckets_ptrs_.begin(), new_bucket);
            } catch (...) {
                (new_bucket + (kBucketSize - 1))->~T();
                RemoveBucket(new_bucket);
                throw;
            }

            first_pos_ = kBucketSize - 1;
            ++last_bucket_;
            ++size_;

            return;
        } else {
            --first_bucket_;
            first_pos_ = kBucketSize - 1;
        }
    } else {
        --first_pos_;
    }

    try {
        new (&(*this)[0]) T(value);
    } catch (...) {
        first_bucket_ = old_first_bucket;
        first_pos_ = old_first_pos;
        throw;
    }

    ++size_;
}

template <typename T>
void Deque<T>::PopBack() {
    if (size_ == 0) {
        return;
    }

    (*this)[size_ - 1].~T();
    if (last_pos_ == 0) {
        --last_bucket_;
        last_pos_ = kBucketSize - 1;
    } else {
        --last_pos_;
    }

    --size_;
}

template <typename T>
void Deque<T>::PopFront() {
    if (size_ == 0) {
        return;
    }

    (*this)[0].~T();
    if (first_pos_ == kBucketSize - 1) {
        first_pos_ = 0;
        ++first_bucket_;
    } else {
        ++first_pos_;
    }

    --size_;
}

template <typename T>
Deque<T>::iterator Deque<T>::begin() noexcept {
    if (buckets_ptrs_.empty()) {
        return iterator();
    }

    return iterator(buckets_ptrs_.data() + first_bucket_, buckets_ptrs_[first_bucket_] + first_pos_,
                    buckets_ptrs_.data() + last_bucket_);
}

template <typename T>
Deque<T>::const_iterator Deque<T>::begin() const noexcept {
    if (buckets_ptrs_.empty()) {
        return const_iterator();
    }

    return const_iterator(const_cast<const T**>(buckets_ptrs_.data() + first_bucket_),
                          const_cast<const T*>(buckets_ptrs_[first_bucket_] + first_pos_),
                          const_cast<const T**>(buckets_ptrs_.data() + last_bucket_));
}

template <typename T>
Deque<T>::const_iterator Deque<T>::cbegin() const noexcept {
    return begin();
}

template <typename T>
Deque<T>::iterator Deque<T>::end() noexcept {
    if (buckets_ptrs_.empty()) {
        return iterator();
    }

    size_t bucket_index = last_bucket_;
    size_t element_index = last_pos_ + 1;
    if (element_index == kBucketSize && bucket_index + 1 < BucketsCount()) {
        ++bucket_index;
        element_index = 0;
    }

    return iterator(buckets_ptrs_.data() + bucket_index,
                    buckets_ptrs_[bucket_index] + element_index,
                    buckets_ptrs_.data() + last_bucket_);
}

template <typename T>
Deque<T>::const_iterator Deque<T>::end() const noexcept {
    if (buckets_ptrs_.empty()) {
        return const_iterator();
    }

    size_t bucket_index = last_bucket_;
    size_t element_index = last_pos_ + 1;
    if (element_index == kBucketSize && bucket_index + 1 < BucketsCount()) {
        ++bucket_index;
        element_index = 0;
    }

    return const_iterator(const_cast<const T**>(buckets_ptrs_.data() + bucket_index),
                          const_cast<const T*>(buckets_ptrs_[bucket_index] + element_index),
                          const_cast<const T**>(buckets_ptrs_.data() + last_bucket_));
}

template <typename T>
Deque<T>::const_iterator Deque<T>::cend() const noexcept {
    return end();
}

template <typename T>
void Deque<T>::Insert(const_iterator it, const T& value) {
    if (it > cend() || it < cbegin()) {
        return;
    }

    if (it == cend()) {
        PushBack(value);
        return;
    }

    if (it == cbegin()) {
        PushFront(value);
        return;
    }

    T pushed_value = *(end() - 1);
    reverse_iterator first = rbegin();
    const_reverse_iterator last(it);

    while ((first + 1) != last) {
        *first = *(first + 1);
        ++first;
    }
    *first = value;

    PushBack(pushed_value);
}

template <typename T>
void Deque<T>::Erase(const_iterator it) {
    if (it > cend() || it < cbegin()) {
        return;
    }

    if (it == cend() - 1) {
        PopBack();
        return;
    }

    if (it == cbegin()) {
        PopFront();
        return;
    }

    iterator first = begin() + (it - begin());
    iterator last = end();
    while (first + 1 != last) {
        *first = *(first + 1);
        ++first;
    }

    PopBack();
}

template <typename T>
void Deque<T>::BackOffConstruction(size_t index) noexcept {
    for (size_t j = 0; j < index; ++j) {
        (*this)[j].~T();
    }
    SetMembersToZero();

    size_t buckets = BucketsCount();
    for (size_t k = 0; k < buckets; ++k) {
        RemoveBucket(buckets_ptrs_[k]);
    }
    buckets_ptrs_.clear();
}

template <typename T>
void Deque<T>::SetMembersToZero() noexcept {
    size_ = first_pos_ = last_pos_ = first_bucket_ = last_bucket_ = 0;
}

template <typename T>
T* Deque<T>::CreateBucket() {
    T* bucket = reinterpret_cast<T*>(new std::byte[sizeof(T) * kBucketSize]);

    return bucket;
}

template <typename T>
void Deque<T>::RemoveBucket(T* bucket_ptr) {
    std::byte* ptr = reinterpret_cast<std::byte*>(bucket_ptr);
    delete[] ptr;
}

template <typename T>
void Deque<T>::AddBucketBack() {
    T* bucket = CreateBucket();
    buckets_ptrs_.push_back(bucket);
}

template <typename T>
void Deque<T>::AddBucketFront() {
    T* bucket = CreateBucket();
    buckets_ptrs_.insert(buckets_ptrs_.begin(), bucket);
}

template <typename T>
void Deque<T>::AddBuckets(size_t size) {
    size_t buckets_to_add = (size + kBucketSize - 1) / kBucketSize;
    for (size_t i = 0; i < buckets_to_add; ++i) {
        AddBucketBack();
    }
}
