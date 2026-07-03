#include <concepts>
#include <cstddef>
#include <functional>
#include <type_traits>

template <typename>
class Function;

template <typename>
class MoveOnlyFunction;

// template <typename R, typename... Args>
// class Function<R(Args...)>;

template <typename>
struct IsFunction : std::false_type {};

template <template <typename, typename...> class F, typename R, typename... Args>
struct IsFunction<F<R(Args...)>> : std::true_type {};

template <typename T>
concept FunctorOrLambda = !IsFunction<T>::value && requires { std::declval<T>(); };

// type deduction for functors and lambda
template <typename... Types>
struct Signature;

// tested is passed without this overload, but let it be
// template <typename C, typename R, typename... Args>
// struct Signature<R (C::*)(Args...)> : Signature<R(Args...)> {};

template <typename C, typename R, typename... Args>
struct Signature<R (C::*)(Args...) const> : Signature<R(Args...)> {};

template <typename R, typename... Args>
struct Signature<R(Args...)> {
    using Type = R(Args...);
};

template <FunctorOrLambda F>
Function(F&&) -> Function<typename Signature<decltype(&F::operator())>::Type>;

template <typename R, typename... Args>
Function(R (*)(Args...)) -> Function<R(Args...)>;

template <typename T>
struct IsInPlaceType : std::false_type {};

template <typename T>
struct IsInPlaceType<std::in_place_type_t<T>> : std::true_type {};

template <typename R, typename... Args>
struct Table {
    void (*CopyOrMove)(void*, void*) noexcept;
    void (*Destroy)(void*) noexcept;
    R (*Invoke)(void*, Args...);
    const std::type_info& (*TargetType)() noexcept;
    void* (*Target)(void*) noexcept;
    const void* (*TargetConst)(const void*) noexcept;
};

template <size_t kBufferSize, typename F, typename R, typename... Args>
struct Methods;

template <size_t kBufferSize, typename F, typename R, typename... Args>
    requires(sizeof(F) <= kBufferSize)
struct Methods<kBufferSize, F, R, Args...> {
    static void CopySpec(void* current_ptr, void* other_ptr) noexcept
        requires(std::is_copy_constructible_v<F>)
    {
        static_cast<F*>(current_ptr)->~F();
        ::new (current_ptr) F(*static_cast<F*>(other_ptr));
    }

    static void MoveSpec(void* current_ptr, void* other_ptr) noexcept {
        ::new (current_ptr) F(std::move(*static_cast<F*>(other_ptr)));
    }

    static void DestroySpec(void* callable_ptr) noexcept {
        static_cast<F*>(callable_ptr)->~F();
    }

    // explanation for C-style pointer to function:
    // we get pointer to pointer to func after "static_cast<F*>"[void (**) (int) - e.g.]
    // and then we dereference it in "*static_cast<F*>"
    static R InvokeSpec(void* callable_ptr, Args... args) {
        return std::invoke(*static_cast<F*>(callable_ptr), std::forward<Args>(args)...);
    }

    static const std::type_info& TargetTypeSpec() noexcept {
        return typeid(F);
    }

    static void* Target(void* callable_ptr) noexcept {
        return callable_ptr;
    }

    static const void* TargetConst(const void* callable_ptr) noexcept {
        return callable_ptr;
    }

    static constexpr Table<R, Args...> kMethodsCopy{&CopySpec,       &DestroySpec, &InvokeSpec,
                                                    &TargetTypeSpec, &Target,      &TargetConst};

    static constexpr Table<R, Args...> kMethodsMove{&MoveSpec,       &DestroySpec, &InvokeSpec,
                                                    &TargetTypeSpec, &Target,      &TargetConst};
};

template <size_t kBufferSize, typename F, typename R, typename... Args>
    requires(sizeof(F) > kBufferSize)
struct Methods<kBufferSize, F, R, Args...> {
    static void CopySpec(void* current_ptr, void* other_ptr) noexcept
        requires(std::is_copy_constructible_v<F>)
    {
        F* other_heap_ptr = *static_cast<F**>(other_ptr);
        F* temp_ptr = ::new F(*other_heap_ptr);
        ::new (current_ptr) F*(temp_ptr);
    }

    static void MoveSpec(void* current_ptr, void* other_ptr) noexcept {
        F* other_heap_ptr = *static_cast<F**>(other_ptr);
        F* temp_ptr = static_cast<F*>(other_heap_ptr);
        other_heap_ptr = nullptr;
        ::new (current_ptr) F*(temp_ptr);
    }

    static void DestroySpec(void* callable_ptr) noexcept {
        ::delete *static_cast<F**>(callable_ptr);
    }

    static R InvokeSpec(void* callable_ptr, Args... args) {
        F** heap_ptr = static_cast<F**>(callable_ptr);
        return std::invoke(**heap_ptr, std::forward<Args>(args)...);
    }

    static const std::type_info& TargetTypeSpec() noexcept {
        return typeid(F);
    }

    static void* Target(void* callable_ptr) noexcept {
        return *static_cast<void**>(callable_ptr);
    }

    static const void* TargetConst(const void* callable_ptr) noexcept {
        return *static_cast<void* const*>(callable_ptr);
    }

    static constexpr Table<R, Args...> kMethodsCopy{&CopySpec,       &DestroySpec, &InvokeSpec,
                                                    &TargetTypeSpec, &Target,      &TargetConst};

    static constexpr Table<R, Args...> kMethodsMove{&MoveSpec,       &DestroySpec, &InvokeSpec,
                                                    &TargetTypeSpec, &Target,      &TargetConst};
};

template <typename R, typename... Args>
class Function<R(Args...)> {
public:
    using ReturnType = R;

    Function() : table_ptr_(nullptr) {
    }

    Function(std::nullptr_t) : Function() {
    }

    Function(const Function& other) : table_ptr_(other.table_ptr_) {
        if (table_ptr_) {
            table_ptr_->CopyOrMove(static_cast<void*>(raw_mem_),
                                   static_cast<void*>(other.raw_mem_));
        }
    }

    Function(Function&& other) noexcept : table_ptr_(other.table_ptr_) {
        if (table_ptr_) {
            table_ptr_->CopyOrMove(static_cast<void*>(raw_mem_),
                                   static_cast<void*>(other.raw_mem_));
            other.table_ptr_->Destroy(static_cast<void*>(other.raw_mem_));
            other.table_ptr_ = nullptr;
        }
    }

    // !std::same_as<std::decay_t<F>, Function<ReturnType(Args...)> serves for
    // correct excluding this ctr for Function
    template <typename F>
    Function(F&& other)
        requires(!std::same_as<std::remove_cvref_t<F>, Function<ReturnType(Args...)>> &&
                 std::is_copy_constructible_v<std::decay_t<F>> &&
                 std::is_invocable_v<std::decay_t<F>, Args...>)
        : table_ptr_(&Methods<kBufferSize, std::decay_t<F>, ReturnType, Args...>::kMethodsCopy) {
        using Fd = std::decay_t<F>;
        if constexpr (sizeof(Fd) <= kBufferSize) {
            ::new (raw_mem_) Fd(std::forward<F>(other));
        } else {
            Fd* callable_ptr = ::new Fd(std::forward<F>(other));
            ::new (raw_mem_) Fd*(callable_ptr);
        }
    }

    ~Function() {
        if (table_ptr_) {
            table_ptr_->Destroy(static_cast<void*>(raw_mem_));
        }
    }

    Function& operator=(const Function& other) {
        if (this == &other) {
            return *this;
        }

        Function temp(other);
        Swap(temp);
        return *this;
    }

    Function& operator=(Function&& other) {
        if (this == &other) {
            return *this;
        }

        Swap(other);
        if (other.table_ptr_) {
            other.table_ptr_->Destroy(static_cast<void*>(other.raw_mem_));
            other.table_ptr_ = nullptr;
        }

        return *this;
    }

    template <typename F>
    Function& operator=(F&& other)
        requires(std::is_invocable_r_v<ReturnType, F, Args...>)
    {
        Function temp(std::forward<F>(other));
        Swap(temp);
        return *this;
    }

    template <typename F>
    Function& operator=(std::reference_wrapper<F> other) noexcept
        requires(std::is_copy_constructible_v<F>)
    {
        Function temp(other.get());
        Swap(temp);
        return *this;
    }

    void Swap(Function& other) noexcept {
        std::swap(table_ptr_, other.table_ptr_);
        std::swap(raw_mem_, other.raw_mem_);
    }

    ReturnType operator()(Args... args) const {
        if (!table_ptr_) {
            throw std::bad_function_call();
        }

        return table_ptr_->Invoke(static_cast<void*>(raw_mem_), std::forward<Args>(args)...);
    }

    explicit operator bool() const noexcept {
        return table_ptr_;
    }

    const std::type_info& TargetType() const noexcept {
        return (table_ptr_) ? table_ptr_->TargetType() : typeid(void);
    }

    template <typename T>
    const T* Target() const noexcept {
        if (!table_ptr_ || typeid(T) != TargetType()) {
            return nullptr;
        }
        return static_cast<const T*>(table_ptr_->TargetConst(static_cast<const void*>(raw_mem_)));
    }

    template <typename T>
    T* Target() noexcept {
        if (!table_ptr_ || typeid(T) != TargetType()) {
            return nullptr;
        }
        return static_cast<T*>(table_ptr_->Target(static_cast<void*>(raw_mem_)));
    }

private:
    static constexpr size_t kBufferSize = 16;

    const Table<ReturnType, Args...>* table_ptr_;
    alignas(std::max_align_t) mutable std::byte raw_mem_[kBufferSize];
};

// unessential, but let it be for symmetry
// template <typename R, typename... Args>
// MoveOnlyFunction(R (*)(Args...)) -> MoveOnlyFunction<R(Args...)>;

template <FunctorOrLambda F>
MoveOnlyFunction(F&&) -> MoveOnlyFunction<typename Signature<decltype(&F::operator())>::Type>;

template <typename F, typename R, typename... Args>
concept MoveAssignAndCtr =
    !std::same_as<std::decay_t<F>, MoveOnlyFunction<R(Args...)>> &&
    !IsInPlaceType<std::decay_t<F>>::value && std::is_invocable_v<std::decay_t<F>, Args...>;

template <typename R, typename... Args>
class MoveOnlyFunction<R(Args...)> {
public:
    using ReturnType = R;

    MoveOnlyFunction() : table_ptr_(nullptr) {
    }

    MoveOnlyFunction(std::nullptr_t) : MoveOnlyFunction() {
    }

    MoveOnlyFunction(const MoveOnlyFunction& other) = delete;

    MoveOnlyFunction(MoveOnlyFunction&& other) noexcept : table_ptr_(other.table_ptr_) {
        if (table_ptr_) {
            table_ptr_->CopyOrMove(static_cast<void*>(raw_mem_),
                                   static_cast<void*>(other.raw_mem_));
            // other.table_ptr_->Destroy(static_cast<void*>(other.raw_mem_));
            other.table_ptr_ = nullptr;
        }
    }

    template <typename F>
    MoveOnlyFunction(F&& other)
        requires(MoveAssignAndCtr<F, R, Args...>)
        : table_ptr_(&Methods<kBufferSize, std::decay_t<F>, ReturnType, Args...>::kMethodsMove) {
        using Fd = std::decay_t<F>;
        if constexpr (sizeof(Fd) <= kBufferSize) {
            ::new (raw_mem_) Fd(std::forward<F>(other));
        } else {
            Fd* callable_ptr = ::new Fd(std::forward<F>(other));
            ::new (raw_mem_) Fd*(callable_ptr);
        }
    }

    ~MoveOnlyFunction() {
        if (table_ptr_) {
            table_ptr_->Destroy(static_cast<void*>(raw_mem_));
        }
    }

    MoveOnlyFunction& operator=(const MoveOnlyFunction& other) = delete;

    MoveOnlyFunction& operator=(MoveOnlyFunction&& other) {
        if (this == &other) {
            return *this;
        }

        Swap(other);
        if (other.table_ptr_) {
            other.table_ptr_->Destroy(static_cast<void*>(other.raw_mem_));
            other.table_ptr_ = nullptr;
        }

        return *this;
    }

    template <typename F>
    MoveOnlyFunction& operator=(F&& other)
        requires(MoveAssignAndCtr<F, R, Args...>)
    {
        MoveOnlyFunction temp(std::forward<F>(other));
        Swap(temp);
        return *this;
    }

    void Swap(MoveOnlyFunction& other) noexcept {
        std::swap(table_ptr_, other.table_ptr_);
        std::swap(raw_mem_, other.raw_mem_);
    }

    ReturnType operator()(Args... args) const {
        if (!table_ptr_) {
            throw std::bad_function_call();
        }

        return table_ptr_->Invoke(static_cast<void*>(raw_mem_), std::forward<Args>(args)...);
    }

    explicit operator bool() const noexcept {
        return table_ptr_;
    }

    const std::type_info& TargetType() const noexcept {
        return (table_ptr_) ? table_ptr_->TargetType() : typeid(void);
    }

    template <typename T>
    const T* Target() const noexcept {
        if (!table_ptr_ || typeid(T) != TargetType()) {
            return nullptr;
        }
        return static_cast<const T*>(table_ptr_->TargetConst(static_cast<const void*>(raw_mem_)));
    }

    template <typename T>
    T* Target() noexcept {
        if (!table_ptr_ || typeid(T) != TargetType()) {
            return nullptr;
        }
        return static_cast<T*>(table_ptr_->Target(static_cast<void*>(raw_mem_)));
    }

private:
    static constexpr size_t kBufferSize = 16;

    const Table<ReturnType, Args...>* table_ptr_;
    alignas(std::max_align_t) mutable std::byte raw_mem_[kBufferSize];
};

template <template <typename, typename...> class F, typename R, typename... ArgTypes>
bool operator==(const F<R(ArgTypes...)>& func, std::nullptr_t) {
    return !static_cast<bool>(func);
}