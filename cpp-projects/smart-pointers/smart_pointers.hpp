#include <memory>
#include <type_traits>

template <typename T>
class EnableSharedFromThis;

template <typename T>
class WeakPtr;

struct BaseControlBlock {
    virtual ~BaseControlBlock() = default;

    virtual void Dispose() noexcept = 0;

    virtual void Destroy() noexcept = 0;

    size_t strong_count_{1};
    size_t weak_count_{0};
};

template <typename T>
class SharedPtr {
public:
    using element_type = T;

    SharedPtr() noexcept : value_ptr_(nullptr), bcb_ptr_(nullptr) {
    }

    // SharedPtr(std::nullptr_t) noexcept : SharedPtr() {
    // }

    template <typename Y>
    explicit SharedPtr(Y* value_ptr)
        : value_ptr_(value_ptr), bcb_ptr_(::new SeparateBlock<Y>(value_ptr)) {
        if constexpr (std::is_base_of_v<EnableSharedFromThis<Y>, Y>) {
            if (value_ptr) {
                value_ptr->weak_this_ = WeakPtr<Y>(*this);
            }
        }
    }

    template <typename Y, typename Deleter,
              typename = std::enable_if_t<!std::is_convertible_v<Deleter, BaseControlBlock*>>>
    SharedPtr(Y* value_ptr, Deleter deleter)
        : value_ptr_(value_ptr),
          bcb_ptr_(::new SeparateCustomDeleter<Y, Deleter>(value_ptr, deleter)) {
        if constexpr (std::is_base_of_v<EnableSharedFromThis<Y>, Y>) {
            if (value_ptr) {
                value_ptr->weak_this_ = WeakPtr<Y>(*this);
            }
        }
    }

    // template <typename Deleter>
    // SharedPtr(std::nullptr_t value_ptr, Deleter deleter)
    //     : value_ptr_(value_ptr), SeparateCustomDeleter(value_ptr, deleter) {
    // }

    template <typename Y, class Deleter, class Alloc>
    SharedPtr(Y* value_ptr, Deleter deleter, Alloc allocator) : value_ptr_(value_ptr) {
        using Block = SeparateFullCustom<Y, Deleter, Alloc>;
        using BlockAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<Block>;

        BlockAlloc block_alloc(allocator);
        Block* bcb_ptr = std::allocator_traits<BlockAlloc>::allocate(block_alloc, 1);
        // std::allocator_traits<BlockAlloc>::construct(block_alloc, bcb_ptr, value_ptr,
        //                                              std::move(deleter), std::move(allocator));
        ::new (bcb_ptr) Block(value_ptr, std::move(deleter), std::move(allocator));
        bcb_ptr_ = bcb_ptr;

        if constexpr (std::is_base_of_v<EnableSharedFromThis<Y>, Y>) {
            if (value_ptr) {
                value_ptr->weak_this_ = WeakPtr<Y>(*this);
            }
        }
    }

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, element_type* ptr) noexcept
        : value_ptr_(ptr), bcb_ptr_(other.bcb_ptr_) {
        TryAddStrong();
    }

    template <typename Y>
    SharedPtr(SharedPtr<Y>&& other, element_type* ptr) noexcept
        : value_ptr_(ptr), bcb_ptr_(other.bcb_ptr_) {
        other.value_ptr_ = nullptr;
        other.bcb_ptr_ = nullptr;
    }

    SharedPtr(const SharedPtr& other) noexcept
        : value_ptr_(other.value_ptr_), bcb_ptr_(other.bcb_ptr_) {
        TryAddStrong();
    }

    template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
    SharedPtr(const SharedPtr<Y>& other) noexcept
        : value_ptr_(other.value_ptr_), bcb_ptr_(other.bcb_ptr_) {
        TryAddStrong();
    }

    SharedPtr(SharedPtr&& other) noexcept : value_ptr_(other.value_ptr_), bcb_ptr_(other.bcb_ptr_) {
        other.value_ptr_ = nullptr;
        other.bcb_ptr_ = nullptr;
    }

    template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
    SharedPtr(SharedPtr<Y>&& other) noexcept
        : value_ptr_(other.value_ptr_), bcb_ptr_(other.bcb_ptr_) {
        other.value_ptr_ = nullptr;
        other.bcb_ptr_ = nullptr;
    }

    ~SharedPtr() {
        DeleteCurrentStrong();
    }

    // cannot use copy-and-swap because constructor is not noexcept
    // but operator= is noexcept on cppreference
    SharedPtr& operator=(const SharedPtr& other) noexcept {
        if (this == &other) {
            return *this;
        }

        CopyAssignImplStrong(other);
        return *this;
    }

    template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
    SharedPtr& operator=(const SharedPtr<Y>& other) noexcept {
        // no need in (this == &other) because it is processing in the first function overload
        CopyAssignImplStrong(other);
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        MoveAssignImplStrong(std::move(other));
        return *this;
    }

    template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
    SharedPtr& operator=(SharedPtr<Y>&& other) noexcept {
        MoveAssignImplStrong(std::move(other));
        return *this;
    }

    void Swap(SharedPtr& other) noexcept {
        std::swap(value_ptr_, other.value_ptr_);
        std::swap(bcb_ptr_, other.bcb_ptr_);
    }

    T& operator*() const noexcept {
        return *Get();
    }

    T* operator->() const noexcept {
        return Get();
    }

    size_t UseCount() const noexcept {
        return (bcb_ptr_) ? bcb_ptr_->strong_count_ : 0;
    }

    void Reset() noexcept {
        DeleteCurrentStrong();
    }

    template <typename Y>
    void Reset(Y* value_ptr) noexcept {
        DeleteCurrentStrong();
        value_ptr_ = value_ptr;
        bcb_ptr_ = ::new SeparateBlock<Y>(value_ptr);
    }

    // typename U since compilator can deduce the type with external definition
    template <typename U, typename... Args>
    friend SharedPtr<U> MakeShared(Args&&... args);

    template <typename U, typename Alloc, typename... Args>
    friend SharedPtr<U> AllocateShared(const Alloc& allocator, Args&&... args);

    element_type* Get() const noexcept {
        return value_ptr_;
    }

private:
    template <typename Y>
    struct SeparateBlock : BaseControlBlock {
        SeparateBlock(Y* value_ptr) : BaseControlBlock(), value_ptr_(value_ptr) {
        }

        void Dispose() noexcept override {
            delete value_ptr_;
        }

        void Destroy() noexcept override {
            delete this;
        }

        Y* value_ptr_;
    };

    // std::enable_if_t<false> - undefined (this will trigger SFINAE)
    template <typename Y, typename Deleter>
    struct SeparateCustomDeleter : BaseControlBlock {
        SeparateCustomDeleter(Y* value_ptr, Deleter deleter)
            : value_ptr_(value_ptr), deleter_(std::move(deleter)) {
        }

        void Dispose() noexcept override {
            deleter_(value_ptr_);
        }

        void Destroy() noexcept override {
            delete this;
        }

        Y* value_ptr_;
        [[no_unique_address]] Deleter deleter_;
    };

    template <typename Y, typename Deleter, typename Alloc>
    struct SeparateFullCustom : BaseControlBlock {
        using BlockAlloc =
            typename std::allocator_traits<Alloc>::template rebind_alloc<SeparateFullCustom>;

        SeparateFullCustom(Y* value_ptr, Deleter deleter, Alloc allocator)
            : BaseControlBlock(),
              value_ptr_(value_ptr),
              deleter_(std::move(deleter)),
              allocator_(std::move(allocator)) {
        }

        void Dispose() noexcept override {
            deleter_(value_ptr_);
        }

        void Destroy() noexcept override {
            auto copy_alloc(allocator_);
            // std::allocator_traits<BlockAlloc>::destroy(copy_alloc, this);
            this->~SeparateFullCustom();
            std::allocator_traits<BlockAlloc>::deallocate(copy_alloc, this, 1);
        }

        Y* value_ptr_;
        [[no_unique_address]] Deleter deleter_;
        [[no_unique_address]] BlockAlloc allocator_;
    };

    template <typename Y>
    struct InPlaceBlock : BaseControlBlock {
        template <typename... Args>
        InPlaceBlock(Args&&... args) {
            ::new (ValuePtr()) Y(std::forward<Args>(args)...);
        }

        void Dispose() noexcept override {
            ValuePtr()->~Y();
        }

        void Destroy() noexcept override {
            delete this;
        }

        const Y* ValuePtr() const noexcept {
            return reinterpret_cast<const Y*>(raw_mem_);
        }

        Y* ValuePtr() noexcept {
            return reinterpret_cast<Y*>(raw_mem_);
        }

        alignas(alignof(Y)) std::byte raw_mem_[sizeof(Y)];
    };

    template <typename Y, typename Alloc>
    struct InPlaceCustomAlloc : BaseControlBlock {
        //  custom allocators will suffer(maybe it is more suitable to contain with no rebind)
        using BlockAlloc =
            typename std::allocator_traits<Alloc>::template rebind_alloc<InPlaceCustomAlloc>;

        template <typename... Args>
        InPlaceCustomAlloc(Alloc allocator, Args&&... args) : allocator_(allocator) {
            std::allocator_traits<Alloc>::construct(allocator_, ValuePtr(),
                                                    std::forward<Args>(args)...);
        }

        void Dispose() noexcept override {
            std::allocator_traits<Alloc>::destroy(allocator_, ValuePtr());
            // ValuePtr()->~Y();
        }

        void Destroy() noexcept override {
            BlockAlloc block_alloc(allocator_);
            // std::allocator_traits<BlockAlloc>::destroy(block_alloc, this);
            this->~InPlaceCustomAlloc();
            std::allocator_traits<BlockAlloc>::deallocate(block_alloc, this, 1);
        }

        const Y* ValuePtr() const noexcept {
            return reinterpret_cast<const Y*>(raw_mem_);
        }

        Y* ValuePtr() noexcept {
            return reinterpret_cast<Y*>(raw_mem_);
        }

        alignas(alignof(Y)) std::byte raw_mem_[sizeof(Y)];
        [[no_unique_address]] Alloc allocator_;
    };

    // BaseControlBlock because we use this constructor for AllocateShared too
    template <typename Y>
    explicit SharedPtr(Y* value_ptr, BaseControlBlock* bcb_ptr) noexcept
        : value_ptr_(value_ptr), bcb_ptr_(bcb_ptr) {
        if constexpr (std::is_base_of_v<EnableSharedFromThis<Y>, Y>) {
            if (value_ptr) {
                value_ptr->weak_this_ = WeakPtr<Y>(*this);
            }
        }
    }

    void DeleteCurrentStrong() noexcept {
        if (!bcb_ptr_) {
            return;
        }

        --bcb_ptr_->strong_count_;
        if (bcb_ptr_->strong_count_ == 0) {
            WeakPtr<T> weak_ptr{*this};
            bcb_ptr_->Dispose();
        }

        value_ptr_ = nullptr;
        bcb_ptr_ = nullptr;
    }

    void TryAddStrong() noexcept {
        if (bcb_ptr_) {
            ++bcb_ptr_->strong_count_;
        }
    }

    template <typename Y>
    void CopyAssignImplStrong(const SharedPtr<Y>& other) noexcept {
        DeleteCurrentStrong();

        value_ptr_ = other.value_ptr_;
        bcb_ptr_ = other.bcb_ptr_;
        TryAddStrong();
    }

    template <typename Y>
    void MoveAssignImplStrong(SharedPtr<Y>&& other) noexcept {
        DeleteCurrentStrong();

        value_ptr_ = other.value_ptr_;
        bcb_ptr_ = other.bcb_ptr_;

        other.value_ptr_ = nullptr;
        other.bcb_ptr_ = nullptr;
    }

    T* value_ptr_;
    BaseControlBlock* bcb_ptr_;

    // for constructors from SharedPtr<Y>(convertible types)
    template <typename U>
    friend class SharedPtr;

    template <typename U>
    friend class WeakPtr;
};

template <typename T>
class WeakPtr {
public:
    WeakPtr() noexcept : value_ptr_(nullptr), bcb_ptr_(nullptr) {
    }

    WeakPtr(const WeakPtr& other) noexcept
        : value_ptr_(other.value_ptr_), bcb_ptr_(other.bcb_ptr_) {
        TryAddWeak();
    }

    template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
    WeakPtr(const WeakPtr<Y>& other) noexcept
        : value_ptr_(other.value_ptr_), bcb_ptr_(other.bcb_ptr_) {
        TryAddWeak();
    }

    template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
    WeakPtr(const SharedPtr<Y>& other) noexcept
        : value_ptr_(other.value_ptr_), bcb_ptr_(other.bcb_ptr_) {
        TryAddWeak();
    }

    WeakPtr(WeakPtr&& other) noexcept : value_ptr_(other.value_ptr_), bcb_ptr_(other.bcb_ptr_) {
        other.value_ptr_ = nullptr;
        other.bcb_ptr_ = nullptr;
    }

    template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
    WeakPtr(WeakPtr<Y>&& other) noexcept : value_ptr_(other.value_ptr_), bcb_ptr_(other.bcb_ptr_) {
        other.value_ptr_ = nullptr;
        other.bcb_ptr_ = nullptr;
    }

    ~WeakPtr() {
        DeleteCurrentWeak();
    }

    WeakPtr& operator=(const WeakPtr& other) {
        if (this == &other) {
            return *this;
        }

        CopyAssignImplWeak(other);
        return *this;
    }

    template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
    WeakPtr& operator=(const WeakPtr<Y>& other) {
        CopyAssignImplWeak(other);
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) {
        if (this == &other) {
            return *this;
        }

        MoveAssignImplWeak(std::move(other));
        return *this;
    }

    template <typename Y, typename = std::enable_if_t<std::is_convertible_v<Y*, T*>>>
    WeakPtr& operator=(WeakPtr<Y>&& other) {
        MoveAssignImplWeak(std::move(other));
        return *this;
    }

    size_t UseCount() const noexcept {
        return (bcb_ptr_) ? bcb_ptr_->strong_count_ : 0;
    }

    bool Expired() const noexcept {
        return (bcb_ptr_) ? bcb_ptr_->strong_count_ == 0 : true;
    }

    SharedPtr<T> Lock() const noexcept {
        if (!bcb_ptr_ || bcb_ptr_->strong_count_ == 0) {
            return SharedPtr<T>();
        }

        ++bcb_ptr_->strong_count_;
        return SharedPtr<T>(value_ptr_, bcb_ptr_);
    }

    const BaseControlBlock* GetBlockPtr() const noexcept {
        return bcb_ptr_;
    }

private:
    void DeleteCurrentWeak() noexcept {
        if (!bcb_ptr_) {
            return;
        }

        --bcb_ptr_->weak_count_;
        if (bcb_ptr_->strong_count_ == 0 && bcb_ptr_->weak_count_ == 0) {
            bcb_ptr_->Destroy();
        }

        bcb_ptr_ = nullptr;
        value_ptr_ = nullptr;
    }

    void TryAddWeak() noexcept {
        if (bcb_ptr_) {
            ++bcb_ptr_->weak_count_;
        }
    }

    template <typename Y>
    void CopyAssignImplWeak(const WeakPtr<Y>& other) {
        DeleteCurrentWeak();

        value_ptr_ = other.value_ptr_;
        bcb_ptr_ = other.bcb_ptr_;
        TryAddWeak();
    }

    template <typename Y>
    void MoveAssignImplWeak(WeakPtr<Y>&& other) {
        DeleteCurrentWeak();

        value_ptr_ = other.value_ptr_;
        bcb_ptr_ = other.bcb_ptr_;

        other.value_ptr_ = nullptr;
        other.bcb_ptr_ = nullptr;
    }

    T* value_ptr_;
    BaseControlBlock* bcb_ptr_;

    template <typename U>
    friend class WeakPtr;
};

template <typename T>
class EnableSharedFromThis {
public:
    EnableSharedFromThis() noexcept = default;

    EnableSharedFromThis(const EnableSharedFromThis& other) noexcept = default;

    EnableSharedFromThis(EnableSharedFromThis&& other) noexcept = default;

    ~EnableSharedFromThis() = default;

    EnableSharedFromThis& operator=(const EnableSharedFromThis&) noexcept {
        return *this;
    }

    EnableSharedFromThis& operator=(EnableSharedFromThis&&) noexcept {
        return *this;
    }

    SharedPtr<T> SharedFromThis() {
        if (!weak_this_.GetBlockPtr()) {
            throw std::bad_weak_ptr();
        }
        return weak_this_.Lock();
    }

    SharedPtr<const T> SharedFromThis() const {
        if (!weak_this_.GetBlockPtr()) {
            throw std::bad_weak_ptr();
        }
        return weak_this_.Lock();
    }

private:
    // enables changing this member in const methods(cppreference)
    mutable WeakPtr<T> weak_this_;

    template <typename U>
    friend class SharedPtr;
};

template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    using Block = typename SharedPtr<T>::template InPlaceBlock<T>;
    Block* bcb_ptr = new Block(std::forward<Args>(args)...);
    return SharedPtr<T>(bcb_ptr->ValuePtr(), bcb_ptr);
}

template <typename T, typename Alloc, typename... Args>
SharedPtr<T> AllocateShared(const Alloc& allocator, Args&&... args) {
    using Block = typename SharedPtr<T>::template InPlaceCustomAlloc<T, Alloc>;
    using BlockAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<Block>;

    BlockAlloc block_allocator(allocator);
    Block* bcb_ptr = std::allocator_traits<BlockAlloc>::allocate(block_allocator, 1);
    // std::allocator_traits<BlockAlloc>::construct(block_allocator, bcb_ptr,
    //                                              allocator, std::forward<Args>(args)...);
    new (bcb_ptr) Block(allocator, std::forward<Args>(args)...);

    return SharedPtr<T>(bcb_ptr->ValuePtr(), bcb_ptr);
}