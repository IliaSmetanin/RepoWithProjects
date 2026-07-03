#include "src/list.hpp"

#include <cmath>
#include <memory>
#include <vector>

template <typename Key, typename Value, typename Hash = std::hash<Key>,
          typename Equal = std::equal_to<Key>,
          typename Alloc = std::allocator<std::pair<const Key, Value>>>
class UnorderedMap {
public:
    using NodeType = std::pair<const Key, Value>;
    using MyList = List<NodeType, Alloc>;

    using KeyType = Key;
    using MappedType = Value;
    using SizeType = size_t;
    using DifferenceType = ptrdiff_t;
    using Hasher = Hash;
    using KeyEqual = Equal;
    using AllocatorType = Alloc;
    using Reference = NodeType&;
    using ConstReference = const NodeType&;
    using Pointer = NodeType*;
    using ConstPointer = const NodeType*;

    using Node = typename MyList::Node;
    using ListAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<MyList>;
    using NodeAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
    using BucketAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<MyList*>;

    using Vector = std::vector<MyList*, BucketAlloc>;

    template <bool IsConst>
    class Iterator {
    public:
        using difference_type = ptrdiff_t;
        using value_type = std::conditional_t<IsConst, const NodeType, NodeType>;
        using pointer = std::conditional_t<IsConst, const NodeType*, NodeType*>;
        using reference = std::conditional_t<IsConst, const NodeType&, NodeType&>;
        using iterator_category = std::forward_iterator_tag;

        using ListIter = typename MyList::template Iterator<IsConst>;
        using MyListPtr = std::conditional_t<IsConst, const MyList*, MyList*>;
        using Buckets = std::conditional_t<IsConst, const Vector, Vector>;

        Iterator() noexcept : buckets_(nullptr), current_list_ptr_(nullptr), list_iter_() {
        }

        Iterator(Buckets* buckets, MyListPtr current_list_ptr, ListIter list_iter) noexcept
            : buckets_(buckets), current_list_ptr_(current_list_ptr), list_iter_(list_iter) {
        }

        Iterator(const Iterator& other) noexcept = default;

        operator Iterator<true>() const noexcept {
            return Iterator<true>(buckets_, current_list_ptr_, list_iter_);
        }

        Iterator& operator=(const Iterator& other) noexcept = default;

        Iterator& operator++() noexcept {
            ++list_iter_;

            if (list_iter_ == current_list_ptr_->end()) {
                current_list_ptr_ = current_list_ptr_->next_not_empty_;
                if (!current_list_ptr_) {
                    list_iter_ = ListIter();
                    return *this;
                }

                list_iter_ = current_list_ptr_->begin();
            }

            return *this;
        }

        Iterator operator++(int) noexcept {
            Iterator temp(*this);
            ++(*this);
            return temp;
        }

        reference operator*() const noexcept {
            return *list_iter_;
        }

        pointer operator->() const noexcept {
            return list_iter_.operator->();
        }

        friend bool operator==(const Iterator& left, const Iterator& right) noexcept {
            return left.list_iter_ == right.list_iter_;
        }

    private:
        Buckets* buckets_;
        MyListPtr current_list_ptr_;
        ListIter list_iter_;

        friend class UnorderedMap;
    };

    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    UnorderedMap() noexcept
        : first_not_empty_(nullptr),
          size_(0),
          max_load_factor_(0.75),
          buckets_(kDefaultBucketCount, nullptr) {
    }

    explicit UnorderedMap(SizeType bucket_count, const Hash& hasher = Hash(),
                          const Equal& equal = Equal(), const Alloc& allocator = Alloc()) noexcept
        : hasher_(hasher),
          equal_(equal),
          allocator_(NodeAlloc(allocator)),
          first_not_empty_(nullptr),
          size_(0),
          max_load_factor_(0.75),
          buckets_(bucket_count == 0 ? kDefaultBucketCount : bucket_count, nullptr) {
    }

    UnorderedMap(SizeType bucket_count, const Alloc& allocator) noexcept
        : UnorderedMap(bucket_count, Hash(), Equal(), allocator) {
    }

    UnorderedMap(SizeType bucket_count, const Hash& hasher, const Alloc& allocator) noexcept
        : UnorderedMap(bucket_count, hasher, Equal(), allocator) {
    }

    explicit UnorderedMap(const Alloc& allocator) noexcept
        : UnorderedMap(kDefaultBucketCount, Hash(), Equal(), allocator) {
    }

    UnorderedMap(const UnorderedMap& other) noexcept
        : UnorderedMap(other,
                       std::allocator_traits<NodeAlloc>::select_on_container_copy_construction(
                           other.allocator_)) {
    }

    UnorderedMap(const UnorderedMap& other, const Alloc& allocator)
        : hasher_(other.hasher_),
          equal_(other.equal_),
          allocator_(NodeAlloc(allocator)),
          first_not_empty_(nullptr),
          size_(other.size_),
          max_load_factor_(other.max_load_factor_) {
        CopyBuckets(other);
    }

    UnorderedMap(UnorderedMap&& other) noexcept
        : hasher_(std::move(other.hasher_)),
          equal_(std::move(other.equal_)),
          allocator_(std::move(other.allocator_)),
          first_not_empty_(other.first_not_empty_),
          size_(other.size_),
          max_load_factor_(other.max_load_factor_),
          buckets_(std::move(other.buckets_)) {
        other.size_ = 0;
        other.buckets_.clear();
    }

    UnorderedMap(UnorderedMap&& other, const Alloc& allocator)
        : hasher_(std::move(other.hasher_)),
          equal_(std::move(other.equal_)),
          allocator_(NodeAlloc(allocator)),
          first_not_empty_(other.first_not_empty_),
          size_(other.size_),
          max_load_factor_(other.max_load_factor_) {
        if (allocator_ == other.allocator_) {
            buckets_ = std::move(other.buckets_);
        } else {
            CopyBuckets(other);
        }
        other.size_ = 0;
        other.buckets_.clear();
    }

    ~UnorderedMap() {
        Clear();
    }

    UnorderedMap& operator=(const UnorderedMap& other) {
        if (this == &other) {
            return *this;
        }

        bool constexpr kPropagateCopy =
            std::allocator_traits<NodeAlloc>::propagate_on_container_copy_assignment::value;
        bool constexpr kPropagateSwap =
            std::allocator_traits<NodeAlloc>::propagate_on_container_swap::value;
        if (kPropagateCopy) {
            UnorderedMap temp(
                other, std::allocator_traits<NodeAlloc>::select_on_container_copy_construction(
                           other.allocator_));
            if (kPropagateSwap) {
                Swap(temp);
            } else {
                SwapContent(temp);
                allocator_ = temp.allocator_;
            }

            return *this;
        }

        UnorderedMap temp(
            other,
            std::allocator_traits<NodeAlloc>::select_on_container_copy_construction(allocator_));
        if (kPropagateSwap) {
            Swap(temp);
        } else {
            SwapContent(temp);
            allocator_ = temp.allocator_;
        }

        return *this;
    }

    UnorderedMap& operator=(UnorderedMap&& other) {
        constexpr bool kPropagateSwap =
            std::allocator_traits<NodeAlloc>::propagate_on_container_swap::value;
        constexpr bool kPropagateMove =
            std::allocator_traits<NodeAlloc>::propagate_on_container_move_assignment::value;

        Clear();
        if (kPropagateMove) {
            if (kPropagateSwap) {
                Swap(other);
                return *this;
            }

            SwapContent(other);
            allocator_ = std::move(other.allocator_);
            return *this;
        }

        SwapContent(other);
        return *this;
    }

    void Swap(UnorderedMap& other) noexcept {
        SwapContent(other);
        bool constexpr kPropagateSwap =
            std::allocator_traits<NodeAlloc>::propagate_on_container_swap::value;
        if (kPropagateSwap) {
            std::swap(allocator_, other.allocator_);
        }
    }

    void Clear() noexcept {
        ListAlloc list_allocator(allocator_);
        for (auto* bucket_ptr : buckets_) {
            DeleteList(bucket_ptr, list_allocator);
        }
        buckets_ = Vector(kDefaultBucketCount, nullptr);
        size_ = 0;
        first_not_empty_ = nullptr;
    }

    SizeType Size() const noexcept {
        return size_;
    }

    bool Empty() const noexcept {
        return size_ == 0;
    }

    iterator Find(const Key& key) {
        return FindImplementation(key, hasher_(key));
    }

    const_iterator Find(const Key& key) const {
        return FindImplementation(key, hasher_(key));
    }

    bool Contains(const Key& key) const noexcept {
        return Find(key) != end();
    }

    Value& operator[](const Key& key) {
        return OperatorBrackets(key);
    }

    Value& operator[](Key&& key) {
        return OperatorBrackets(std::move(key));
    }

    Value& At(const Key& key) {
        iterator it = Find(key);
        if (it == end()) {
            throw std::out_of_range("Method At() has thrown an exception: key not found");
        }

        return it->second;
    }

    const Value& At(const Key& key) const {
        const_iterator it = Find(key);
        if (it == cend()) {
            throw std::out_of_range("Method At() has thrown an exception: key not found");
        }

        return it->second;
    }

    template <typename... Args>
    std::pair<iterator, bool> Emplace(Args&&... args) {
        Node* node_ptr = std::allocator_traits<NodeAlloc>::allocate(allocator_, 1);

        // ::new (static_cast<void*>(node_ptr)) Node();
        std::allocator_traits<NodeAlloc>::construct(allocator_, node_ptr);

        NodeType* value_ptr = node_ptr->ValuePtr();
        Alloc alloc(allocator_);
        // exceptions is thrown by hasher_ and constructor and
        // first case requires only to destroy, second - to destroy and deallocate
        try {
            std::allocator_traits<Alloc>::construct(alloc, value_ptr, std::forward<Args>(args)...);
        } catch (...) {
            std::allocator_traits<NodeAlloc>::destroy(allocator_, node_ptr);
            std::allocator_traits<NodeAlloc>::deallocate(allocator_, node_ptr, 1);
            throw;
        }

        const Key& key = node_ptr->ValuePtr()->first;
        try {
            node_ptr->hash_ = hasher_(key);
        } catch (...) {
            std::allocator_traits<Alloc>::destroy(alloc, value_ptr);
            std::allocator_traits<NodeAlloc>::destroy(allocator_, node_ptr);
            std::allocator_traits<NodeAlloc>::deallocate(allocator_, node_ptr, 1);
            throw;
        }

        auto it = FindImplementation(key, node_ptr->hash_);
        if (it != end()) {
            std::allocator_traits<Alloc>::destroy(alloc, value_ptr);
            std::allocator_traits<NodeAlloc>::destroy(allocator_, node_ptr);
            std::allocator_traits<NodeAlloc>::deallocate(allocator_, node_ptr, 1);
            return {it, false};
        }

        TryRehash();

        if (buckets_.empty()) {
            buckets_ = Vector(kDefaultBucketCount, nullptr);
        }

        const size_t bucket_for_emplace = node_ptr->hash_ % buckets_.size();
        bool bucket_was_null = !buckets_[bucket_for_emplace];
        CheckList(bucket_for_emplace);
        MyList& current_list = *buckets_[bucket_for_emplace];
        current_list.PushFrontNode(node_ptr);
        if (bucket_was_null) {
            UpdateFirstNotEmpty(buckets_[bucket_for_emplace]);
        }
        ++size_;

        iterator final(&buckets_, buckets_[bucket_for_emplace], current_list.begin());
        return {final, true};
    }

    // for brace-init-list value-type of that couldn't be deduced
    std::pair<iterator, bool> Insert(const NodeType& node) {
        return Emplace(node);
    }

    template <typename Pair>
    std::pair<iterator, bool> Insert(Pair&& pair) {
        return Emplace(std::forward<Pair>(pair));
    }

    template <typename InputIterator>
    void Insert(InputIterator first, InputIterator last) {
        while (first != last) {
            Insert(*first);
            ++first;
        }
    }

    iterator Erase(iterator it) {
        if (size_ == 0 || it == end() || &buckets_ != it.buckets_) {
            return end();
        }

        auto next = it;
        ++next;

        MyList* current_list_ptr = it.current_list_ptr_;
        current_list_ptr->Erase(it.list_iter_);

        if (current_list_ptr->Empty()) {
            if (current_list_ptr->prev_not_empty_) {
                current_list_ptr->prev_not_empty_->next_not_empty_ =
                    current_list_ptr->next_not_empty_;
            } else {
                first_not_empty_ = current_list_ptr->next_not_empty_;
            }

            if (current_list_ptr->next_not_empty_) {
                current_list_ptr->next_not_empty_->prev_not_empty_ =
                    current_list_ptr->prev_not_empty_;
            }

            current_list_ptr->next_not_empty_ = nullptr;
            current_list_ptr->prev_not_empty_ = nullptr;
        }

        --size_;
        return next;
    }

    iterator Erase(iterator first, iterator last) {
        auto next = last;
        if (next != end()) {
            ++next;
        }

        while (first != last) {
            auto to_erase = first;
            ++first;
            Erase(to_erase);
        }

        return next;
    }

    iterator begin() noexcept {
        if (!first_not_empty_) {
            return end();
        }

        return iterator(&buckets_, first_not_empty_, first_not_empty_->begin());
    }

    const_iterator begin() const noexcept {
        return cbegin();
    }

    const_iterator cbegin() const noexcept {
        if (!first_not_empty_) {
            return cend();
        }

        return const_iterator(&buckets_, first_not_empty_, first_not_empty_->begin());
    }

    iterator end() noexcept {
        return iterator(&buckets_, nullptr, typename MyList::iterator());
    }

    const_iterator end() const noexcept {
        return cend();
    }

    const_iterator cend() const noexcept {
        return const_iterator(&buckets_, nullptr, typename MyList::const_iterator());
    }

    void Rehash(SizeType count) {
        if (count == 0 || count < buckets_.size()) {
            return;
        }
        Vector new_buckets(count, nullptr);

        ListAlloc list_allocator(allocator_);
        for (auto* list : buckets_) {
            if (!list) {
                continue;
            }

            while (!list->Empty()) {
                Node* to_move = list->PopFrontNode();
                const size_t hashed_bucket = to_move->hash_ % count;
                if (!new_buckets[hashed_bucket]) {
                    new_buckets[hashed_bucket] = CreateEmptyList(list_allocator);
                }

                new_buckets[hashed_bucket]->PushFrontNode(to_move);
            }

            DeleteList(list, list_allocator);
        }
        buckets_ = std::move(new_buckets);
        first_not_empty_ = nullptr;
        for (auto* current_list_ptr : buckets_) {
            if (current_list_ptr) {
                UpdateFirstNotEmpty(current_list_ptr);
            }
        }
    }

    void Reserve(SizeType count) {
        double total_amount = static_cast<double>(count) + static_cast<double>(size_);
        const size_t new_buckets = static_cast<size_t>(std::ceil(total_amount / max_load_factor_));

        if (new_buckets > buckets_.size()) {
            Rehash(new_buckets);
        }
    }

    double LoadFactor() const {
        if (buckets_.empty()) {
            return 0;
        }
        return static_cast<double>(size_) / static_cast<double>(buckets_.size());
    }

    double MaxLoadFactor() const noexcept {
        return max_load_factor_;
    }

    void MaxLoadFactor(float max_load_factor) {
        max_load_factor_ = max_load_factor;
    }

    Alloc GetAllocator() const noexcept {
        return Alloc(allocator_);
    }

private:
    static const size_t kDefaultBucketCount = 7;  // prime num for hash-func

    static MyList* CreateEmptyList(ListAlloc& list_allocator) {
        MyList* new_list = std::allocator_traits<ListAlloc>::allocate(list_allocator, 1);
        std::allocator_traits<ListAlloc>::construct(list_allocator, new_list);
        new_list->next_not_empty_ = nullptr;
        new_list->prev_not_empty_ = nullptr;

        return new_list;
    }

    static MyList* CreateCopyList(const MyList* bucket_ptr, ListAlloc& list_allocator) {
        if (!bucket_ptr) {
            return nullptr;
        }
        MyList* new_list = std::allocator_traits<ListAlloc>::allocate(list_allocator, 1);
        std::allocator_traits<ListAlloc>::construct(list_allocator, new_list, *bucket_ptr);
        new_list->next_not_empty_ = nullptr;
        new_list->prev_not_empty_ = nullptr;

        return new_list;
    }

    static void DeleteList(MyList* bucket_ptr, ListAlloc& list_allocator) noexcept {
        if (!bucket_ptr) {
            return;
        }
        std::allocator_traits<ListAlloc>::destroy(list_allocator, bucket_ptr);
        std::allocator_traits<ListAlloc>::deallocate(list_allocator, bucket_ptr, 1);
    }

    void SwapContent(UnorderedMap& other) noexcept {
        std::swap(hasher_, other.hasher_);
        std::swap(equal_, other.equal_);
        std::swap(first_not_empty_, other.first_not_empty_);
        std::swap(size_, other.size_);
        std::swap(max_load_factor_, other.max_load_factor_);
        std::swap(buckets_, other.buckets_);
    }

    void CopyBuckets(const UnorderedMap& other) {
        buckets_.reserve(other.buckets_.size());
        ListAlloc list_allocator(allocator_);

        try {
            for (auto* bucket_ptr : other.buckets_) {
                MyList* new_list = CreateCopyList(bucket_ptr, list_allocator);
                buckets_.push_back(new_list);
                allocator_ = NodeAlloc(list_allocator);
            }
        } catch (...) {
            for (auto* bucket_ptr : buckets_) {
                DeleteList(bucket_ptr, list_allocator);
                allocator_ = NodeAlloc(list_allocator);
            }
            buckets_.clear();

            throw;
        }

        first_not_empty_ = nullptr;
        for (auto* current_list_ptr : buckets_) {
            if (current_list_ptr) {
                UpdateFirstNotEmpty(current_list_ptr);
            }
        }
    }

    template <typename Arg>
    Value& OperatorBrackets(Arg&& key) {
        auto pair = Emplace(std::forward<Arg>(key), Value{});
        return pair.first->second;
    }

    void TryRehash() {
        if (size_ + 1 > max_load_factor_ * buckets_.size()) {
            Reserve(size_ + 1);
        }
    }

    void CheckList(size_t index) {
        if (!buckets_[index]) {
            ListAlloc list_allocator(allocator_);
            buckets_[index] = CreateEmptyList(list_allocator);
        }
    }

    void UpdateFirstNotEmpty(MyList* current_list_ptr) {
        current_list_ptr->next_not_empty_ = first_not_empty_;
        if (first_not_empty_) {
            first_not_empty_->prev_not_empty_ = current_list_ptr;
        }

        first_not_empty_ = current_list_ptr;
    }

    iterator FindImplementation(const Key& key, size_t hash) {
        const size_t hash_bucket = hash % buckets_.size();
        if (!buckets_[hash_bucket]) {
            return end();
        }

        MyList& current_list = *buckets_[hash_bucket];
        for (auto it = current_list.begin(); it != current_list.end(); ++it) {
            if (equal_(it->first, key)) {
                return iterator(&buckets_, buckets_[hash_bucket], it);
            }
        }

        return end();
    }

    const_iterator FindImplementation(const Key& key, size_t hash) const {
        const size_t hash_bucket = hash % buckets_.size();
        if (!buckets_[hash_bucket]) {
            return cend();
        }

        const MyList& current_list = *buckets_[hash_bucket];
        for (auto it = current_list.cbegin(); it != current_list.cend(); ++it) {
            if (equal_(it->first, key)) {
                return const_iterator(&buckets_, buckets_[hash_bucket], it);
            }
        }

        return cend();
    }

    [[no_unique_address]] Hash hasher_;
    [[no_unique_address]] Equal equal_;
    [[no_unique_address]] NodeAlloc allocator_;
    MyList* first_not_empty_;
    SizeType size_;
    double max_load_factor_;
    Vector buckets_;
};