#pragma once

#include <cstddef>
#include <memory>

template <typename T, typename Allocator = std::allocator<T>>
class List {
public:
    struct BaseNode {
        BaseNode() : prev_(this), next_(this) {
        }

        BaseNode* prev_;
        BaseNode* next_;
    };

    struct Node : BaseNode {
        T* ValuePtr() {
            return reinterpret_cast<T*>(raw_mem_);
        }

        const T* ValuePtr() const {
            return reinterpret_cast<const T*>(raw_mem_);
        }

        // uint16_t hash_{};
        alignas(T) std::byte raw_mem_[sizeof(T)];
    };

    template <bool IsConst>
    class Iterator {
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = std::conditional_t<IsConst, const T, T>;
        using pointer = std::conditional_t<IsConst, const T*, T*>;
        using reference = std::conditional_t<IsConst, const T&, T&>;
        using iterator_category = std::bidirectional_iterator_tag;

        using BaseNodeType = std::conditional_t<IsConst, const BaseNode, BaseNode>;
        using NodeType = std::conditional_t<IsConst, const Node, Node>;

        Iterator() noexcept : node_ptr_(nullptr) {
        }

        Iterator(BaseNodeType* node_ptr) noexcept : node_ptr_(node_ptr) {
        }

        Iterator(const Iterator& right) noexcept : node_ptr_(right.node_ptr_) {
        }

        operator Iterator<true>() const noexcept {
            return Iterator<true>(node_ptr_);
        }

        Iterator& operator=(const Iterator& right) noexcept = default;

        Iterator& operator++() noexcept {
            node_ptr_ = node_ptr_->next_;
            return *this;
        }

        Iterator operator++(int) noexcept {
            Iterator temp(*this);
            ++(*this);
            return temp;
        }

        Iterator& operator--() noexcept {
            node_ptr_ = node_ptr_->prev_;
            return *this;
        }

        Iterator operator--(int) noexcept {
            Iterator temp(*this);
            --(*this);
            return temp;
        }

        reference operator*() const noexcept {
            return *static_cast<NodeType*>(node_ptr_)->ValuePtr();
        }

        pointer operator->() const noexcept {
            return static_cast<NodeType*>(node_ptr_)->ValuePtr();
        }

        friend bool operator==(const Iterator& left, const Iterator& right) {
            return left.node_ptr_ == right.node_ptr_;
        }

    private:
        friend class List;  // in order not to write public getter and violate incapsulation
        BaseNodeType* node_ptr_;
    };

    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using value_type = T;

    using NodeAlloc = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;

    List() noexcept : List(Allocator()) {
    }

    explicit List(const Allocator& allocator) noexcept
        : allocator_(NodeAlloc(allocator)), size_(0), sentinel_() {
    }

    List(size_t size, const T& value, const Allocator& allocator = Allocator()) : List(allocator) {
        for (size_t i = 0; i < size; ++i) {
            EmplaceBack(value);
        }
    }

    explicit List(size_t size, const Allocator& allocator = Allocator()) : List(allocator) {
        for (size_t i = 0; i < size; ++i) {
            EmplaceBack();
        }
    }

    List(const List& right)
        : List(right, std::allocator_traits<Allocator>::select_on_container_copy_construction(
                          right.allocator_)) {
    }

    List(const List& right, const Allocator& allocator) : List(right, NodeAlloc(allocator)) {
    }

    List(List&& right) : allocator_(std::move(right.allocator_)), size_(right.size_) {
        if (right.sentinel_.next_ == &right.sentinel_) {
            sentinel_.next_ = &sentinel_;
            sentinel_.prev_ = &sentinel_;
        } else {
            sentinel_.next_ = right.sentinel_.next_;
            sentinel_.prev_ = right.sentinel_.prev_;

            right.sentinel_.prev_->next_ = &sentinel_;
            right.sentinel_.next_->prev_ = &sentinel_;
        }

        right.size_ = 0;
        right.sentinel_.next_ = &right.sentinel_;
        right.sentinel_.prev_ = &right.sentinel_;
    }

    ~List() {
        Clear();
    }

    List& operator=(const List& right) {
        if (this == &right) {
            return *this;
        }

        if constexpr (std::allocator_traits<
                          Allocator>::propagate_on_container_copy_assignment::value) {
            List temp(right, right.allocator_);
            SwapContent(temp);
            allocator_ = temp.allocator_;
            return *this;
        }

        List temp(right, std::allocator_traits<Allocator>::select_on_container_copy_construction(
                             allocator_));
        SwapContent(temp);
        allocator_ = temp.allocator_;

        return *this;
    }

    List& operator=(List&& right) {
        if constexpr (std::allocator_traits<
                          Allocator>::propagate_on_container_move_assignment::value) {
            if constexpr (std::allocator_traits<Allocator>::propagate_on_container_swap::value) {
                // Clear();  // excessive
                Swap(right);
                return *this;
            }

            Clear();
            SwapContent(right);
            allocator_ = std::move(right.allocator_);

            return *this;
        }

        // Clear(); // excessive
        SwapContent(right);
        return *this;
    }

    void Swap(List& right) noexcept {
        SwapContent(right);
        if constexpr (std::allocator_traits<Allocator>::propagate_on_container_swap::value) {
            std::swap(allocator_, right.allocator_);
        }
    }

    void PushBack(const T& value) {
        Node* ptr = CreateFromPack(value);
        PlaceBack(ptr);
        ++size_;
    }

    void PushBack(T&& value) {
        Node* ptr = CreateFromPack(std::move(value));
        PlaceBack(ptr);
        ++size_;
    }

    void PushPushBackNode(Node* to_push) {
        PlacePushBack(to_push);
        ++size_;
    }

    void PushFront(const T& value) {
        Node* ptr = CreateFromPack(value);
        PlaceFront(ptr);
        ++size_;
    }

    void PushFront(T&& value) {
        Node* ptr = CreateFromPack(std::move(value));
        PlaceFront(ptr);
        ++size_;
    }

    void PushFrontNode(Node* to_push) {
        PlaceFront(to_push);
        ++size_;
    }

    void PopBack() {
        if (size_ == 0) {
            return;
        }

        Node* to_delete = static_cast<Node*>(sentinel_.prev_);
        to_delete->prev_->next_ = &sentinel_;
        sentinel_.prev_ = to_delete->prev_;

        DeleteNode(to_delete);
        --size_;
    }

    Node* PopBackNode() {
        if (size_ == 0) {
            return nullptr;
        }

        return ExtractBack();
    }

    void PopFront() {
        if (size_ == 0) {
            return;
        }

        Node* to_delete = ExtractFront();

        DeleteNode(to_delete);
    }

    Node* PopFrontNode() {
        if (size_ == 0) {
            return nullptr;
        }

        return ExtractFront();
    }

    template <typename... Args>
    void EmplaceBack(Args&&... args) {
        PlaceBack(CreateFromPack(std::forward<Args>(args)...));
        ++size_;
    }

    template <typename... Args>
    void EmplaceFront(Args&&... args) {
        PlaceFront(CreateFromPack(std::forward<Args>(args)...));
        ++size_;
    }

    Allocator GetAllocator() const noexcept {
        return Allocator(allocator_);
    }

    size_t Size() const noexcept {
        return size_;
    }

    bool Empty() const noexcept {
        return size_ == 0;
    }

    void Clear() {
        Node* to_delete = static_cast<Node*>(sentinel_.next_);
        while (to_delete != &sentinel_) {
            Node* temp = static_cast<Node*>(to_delete->next_);
            DeleteNode(to_delete);
            to_delete = temp;
        }

        sentinel_.next_ = &sentinel_;
        sentinel_.prev_ = &sentinel_;
        size_ = 0;
    }

    iterator begin() noexcept {
        return iterator(sentinel_.next_);
    }

    const_iterator begin() const noexcept {
        return const_iterator(sentinel_.next_);
    }

    const_iterator cbegin() const noexcept {
        return const_iterator(sentinel_.next_);
    }

    iterator end() noexcept {
        return iterator(&sentinel_);
    }

    const_iterator end() const noexcept {
        return const_iterator(&sentinel_);
    }

    const_iterator cend() const noexcept {
        return const_iterator(&sentinel_);
    }

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

    template <typename... Args>
    iterator Insert(const_iterator it, Args&&... args) {
        Node* ptr = CreateFromPack(std::forward<Args>(args)...);
        BaseNode* pos_node = const_cast<BaseNode*>(it.node_ptr_);
        BaseNode* left = pos_node->prev_;

        ptr->prev_ = left;
        ptr->next_ = left->next_;
        ptr->prev_->next_ = ptr;
        ptr->next_->prev_ = ptr;

        ++size_;

        return iterator(ptr);
    }

    void Erase(const_iterator it) noexcept {
        if (size_ == 0 || it.node_ptr_ == &sentinel_) {
            return;
        }

        BaseNode* pos_node = const_cast<BaseNode*>(it.node_ptr_);

        BaseNode* left = pos_node->prev_;
        BaseNode* right = pos_node->next_;
        Node* to_delete = static_cast<Node*>(left->next_);

        left->next_ = right;
        right->prev_ = left;
        DeleteNode(to_delete);

        --size_;
    }

    bool operator==(const List& right) const {
        if (this == &right) {
            return true;
        }

        if (size_ != right.size_) {
            return false;
        }

        const_iterator it = cbegin();
        const_iterator right_it = right.cbegin();
        while (it != cend()) {
            if (*it != *right_it) {
                return false;
            }
            ++it;
            ++right_it;
        }

        return true;
    }

private:
    List(const List& right, const NodeAlloc& allocator)
        : allocator_(allocator), size_(0), sentinel_() {
        try {
            BaseNode* right_node = right.sentinel_.next_;
            while (right_node != &right.sentinel_) {
                PushBack(*static_cast<Node*>(right_node)->ValuePtr());
                right_node = right_node->next_;
            }
        } catch (...) {
            Clear();
            throw;
        }
    }

    void SwapContent(List& right) {
        std::swap(sentinel_.prev_->next_, right.sentinel_.prev_->next_);
        std::swap(sentinel_.next_->prev_, right.sentinel_.next_->prev_);
        std::swap(sentinel_.next_, right.sentinel_.next_);
        std::swap(sentinel_.prev_, right.sentinel_.prev_);
        std::swap(size_, right.size_);
    }

    // helping methods for EmplaceBack/Front
    template <typename... Args>
    Node* CreateFromPack(Args&&... args) {
        Node* node_ptr = std::allocator_traits<NodeAlloc>::allocate(allocator_, 1);

        Allocator alloc(allocator_);
        T* t_ptr{nullptr};

        try {
            ::new (static_cast<void*>(node_ptr)) Node();
            t_ptr = node_ptr->ValuePtr();
            std::allocator_traits<Allocator>::construct(alloc, t_ptr, std::forward<Args>(args)...);
        } catch (...) {
            if (t_ptr) {
                node_ptr->~Node();
            }
            std::allocator_traits<NodeAlloc>::deallocate(allocator_, node_ptr, 1);

            throw;
        }

        return node_ptr;
    }

    void PlaceBack(Node* ptr) noexcept {
        ptr->prev_ = sentinel_.prev_;
        ptr->next_ = &sentinel_;
        sentinel_.prev_->next_ = ptr;
        sentinel_.prev_ = ptr;
    }

    void PlaceFront(Node* ptr) noexcept {
        ptr->next_ = sentinel_.next_;
        ptr->prev_ = &sentinel_;
        sentinel_.next_->prev_ = ptr;
        sentinel_.next_ = ptr;
    }

    void DeleteNode(Node* to_delete) {
        if (!to_delete) {
            return;
        }
        Allocator alloc(allocator_);
        T* t_ptr{nullptr};

        Node* node_ptr = to_delete;
        t_ptr = node_ptr->ValuePtr();
        std::allocator_traits<Allocator>::destroy(alloc, t_ptr);
        node_ptr->~Node();

        std::allocator_traits<NodeAlloc>::deallocate(allocator_, to_delete, 1);
    }

    Node* ExtractBack() {
        if (size_ == 0) {
            return nullptr;
        }
        Node* to_extract = static_cast<Node*>(sentinel_.prev_);
        to_extract->prev_->next_ = &sentinel_;
        sentinel_.prev_ = to_extract->prev_;
        --size_;

        return to_extract;
    }

    Node* ExtractFront() {
        if (size_ == 0) {
            return nullptr;
        }
        Node* to_extract = static_cast<Node*>(sentinel_.next_);
        to_extract->next_->prev_ = &sentinel_;
        sentinel_.next_ = to_extract->next_;
        --size_;

        return to_extract;
    }

    [[no_unique_address]] NodeAlloc allocator_;
    size_t size_;
    BaseNode sentinel_;
};