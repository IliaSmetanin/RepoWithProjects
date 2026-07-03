#include <cstddef>
#include <type_traits>
#include <utility>

template <typename...>
class Tuple;

// deduction guide
template <typename E1, typename E2>
Tuple(const std::pair<E1, E2>&) -> Tuple<E1, E2>;

template <typename E1, typename E2>
Tuple(std::pair<E1, E2>&&) -> Tuple<E1, E2>;

template <typename... UTypes>
    requires(sizeof...(UTypes) > 0)
Tuple(UTypes...) -> Tuple<UTypes...>;

template <size_t Index, typename Head, typename... Tail>
struct TupleImpl : TupleImpl<Index + 1, Tail...> {
    TupleImpl()
        requires(std::is_default_constructible_v<Head>)
        : TupleImpl<Index + 1, Tail...>(), value_() {
    }

    TupleImpl(const Head& value, const Tail&... args)
        requires(std::is_copy_constructible_v<Head>)
        : TupleImpl<Index + 1, Tail...>(args...), value_(value) {
    }

    template <typename UHead, typename... UTail>
    TupleImpl(UHead&& value, UTail&&... args)
        requires(std::is_constructible_v<Head, UHead>)
        : TupleImpl<Index + 1, Tail...>(std::forward<UTail>(args)...),
          value_(std::forward<UHead>(value)) {
    }

    template <typename UHead, typename... UTail>
    TupleImpl(const TupleImpl<Index, UHead, UTail...>& other)
        : TupleImpl<Index + 1, Tail...>(other), value_(other.value_) {
    }

    // Move-ctr of base class interacts only with the respective fields and
    // doesn't move other fields(like "other.value_" below)
    template <typename UHead, typename... UTail>
    TupleImpl(TupleImpl<Index, UHead, UTail...>&& other)
        : TupleImpl<Index + 1, Tail...>(std::move(other)),
          value_(std::forward<UHead>(other.value_)) {
    }

    TupleImpl(const TupleImpl&) = default;

    TupleImpl(TupleImpl&&) = default;

    ~TupleImpl() {
    }

    TupleImpl& operator=(const TupleImpl& other) {
        value_ = other.value_;
        static_cast<TupleImpl<Index + 1, Tail...>&>(*this) = other;
        return *this;
    }

    TupleImpl& operator=(TupleImpl&& other) {
        value_ = std::move(other.value_);
        static_cast<TupleImpl<Index + 1, Tail...>&>(*this) = std::move(other);
        return *this;
    }

    template <typename UHead, typename... UTail>
    TupleImpl& operator=(const TupleImpl<Index, UHead, UTail...>& other) {
        value_ = other.value_;
        static_cast<TupleImpl<Index + 1, Tail...>&>(*this) = other;
        return *this;
    }

    template <typename UHead, typename... UTail>
    TupleImpl& operator=(TupleImpl<Index, UHead, UTail...>&& other) {
        value_ = std::forward<UHead>(other.value_);
        static_cast<TupleImpl<Index + 1, Tail...>&>(*this) = std::move(other);
        return *this;
    }

    // void Swap(TupleImpl& other) {
    //     std::swap(value_, other.value_);
    //     static_cast<TupleImpl<Index + 1, Tail...>&>(*this).Swap(other);
    // }

    Head value_;
};

template <size_t Index, typename Head>
struct TupleImpl<Index, Head> {
    TupleImpl()
        requires(std::is_default_constructible_v<Head>)
        : value_() {
    }

    TupleImpl(const Head& value)
        requires(std::is_copy_constructible_v<Head>)
        : value_(value) {
    }

    template <typename UHead>
    TupleImpl(UHead&& value)
        requires(std::is_constructible_v<Head, UHead>)
        : value_(std::forward<UHead>(value)) {
    }

    template <typename UHead>
    TupleImpl(const TupleImpl<Index, UHead>& other) : value_(other.value_) {
    }

    template <typename UHead>
    TupleImpl(TupleImpl<Index, UHead>&& other) : value_(std::move(other.value_)) {
    }

    TupleImpl(const TupleImpl&) = default;

    TupleImpl(TupleImpl&&) = default;

    ~TupleImpl() {
    }

    TupleImpl& operator=(const TupleImpl& other) {
        value_ = other.value_;
        return *this;
    }

    TupleImpl& operator=(TupleImpl&& other) {
        value_ = std::move(other.value_);
        return *this;
    }

    template <typename UHead>
    TupleImpl& operator=(const TupleImpl<Index, UHead>& other) {
        value_ = other.value_;
        return *this;
    }

    template <typename UHead>
    TupleImpl& operator=(TupleImpl<Index, UHead>&& other) {
        value_ = std::move(other.value_);
        return *this;
    }

    // void Swap(TupleImpl& other) {
    //     std::swap(value_, other.value_);
    // }

    Head value_;
};

template <typename... Types>
class Tuple : public TupleImpl<0, Types...> {
public:
    // traits for ctr from "std::pair"
    template <typename... UTypes>
    struct CopyConstructibleDeduction
        : std::integral_constant<bool, (std::is_constructible_v<Types, const UTypes&> && ...)> {};

    template <typename... UTypes>
    struct MoveConstructibleDeduction
        : std::integral_constant<bool, (std::is_constructible_v<Types, UTypes&&> && ...)> {};

    template <typename... UTypes>
    struct LvalConvertibleDeduction
        : std::integral_constant<bool, (std::is_convertible_v<const UTypes&, Types> && ...)> {};

    template <typename... UTypes>
    struct RvalConvertibleDeduction
        : std::integral_constant<bool, (std::is_convertible_v<UTypes&&, Types> && ...)> {};

    // traits for operator= from "std::pair"
    template <typename... UTypes>
    struct IsPairCopyAssignable
        : std::integral_constant<bool, (std::is_assignable_v<const Types&, const UTypes&> && ...)> {
    };

    // no && in "std::is_assignable" since it uses std::declval and turn E1 into E1&&
    template <typename... UTypes>
    struct IsPairMoveAssignable
        : std::integral_constant<bool, (std::is_assignable_v<Types&, UTypes> && ...)> {};

    // "copy-list-initializable" is equivalent to "T t = {}" for default ctr
    // '=' is crucial for T with explicit ctr
    explicit((!requires { CheckImplicit<Types>({}); } || ...)) Tuple()
        requires(std::is_default_constructible_v<Types> && ...)
    {
    }

    // second "()" is for fold expr
    explicit((!std::is_convertible_v<const Types&, Types> || ...)) Tuple(const Types&... args)
        requires(std::is_copy_constructible_v<Types> && ...)
        : TupleImpl<0, Types...>(args...) {
    }

    // "std::is_constructible_v<Types, UTypes>" - no "&&" in "UTypes&&"
    // since declval returns rvalue-ref
    template <typename... UTypes>
    explicit((!std::is_convertible_v<UTypes&&, Types> || ...)) Tuple(UTypes&&... args)
        requires((sizeof...(Types) == sizeof...(UTypes)) && sizeof...(Types) > 1 &&
                 (std::is_constructible_v<Types, UTypes> && ...))
        : TupleImpl<0, Types...>(std::forward<UTypes>(args)...) {
    }

    // We suppose that Types... and UTypes... expands to T and U respectively in conditions after
    // "sizeof...(Types) != 1".
    // We must check conversions to avoid facing conflicts with
    // Tuple(const UTypes&...) and Tuple(UTypes&&...).
    // Besides, we must check std::same_as<T, U> to avoid facing conflicts with default copy-ctr.
    template <typename... UTypes>
    explicit((!std::is_convertible_v<const UTypes&, Types> || ...))
        Tuple(const Tuple<UTypes...>& other)
        requires(sizeof...(Types) == sizeof...(UTypes) &&
                 (std::is_constructible_v<Types, const UTypes&> && ...) &&
                 (sizeof...(Types) != 1 ||
                  ((!std::is_constructible_v<Types, decltype(other)> && ...) &&
                   (!std::is_convertible_v<decltype(other), Types> && ...) &&
                   (!std::same_as<Types, UTypes> && ...))))
        : TupleImpl<0, Types...>(other) {
    }

    template <typename... UTypes>
    explicit((!std::is_convertible_v<UTypes&&, Types> || ...)) Tuple(Tuple<UTypes...>&& other)
        requires(sizeof...(Types) == sizeof...(UTypes) &&
                 (std::is_constructible_v<Types, UTypes> && ...) &&
                 (sizeof...(Types) != 1 ||
                  ((!std::is_constructible_v<Types, decltype(other)> && ...) &&
                   (!std::is_convertible_v<decltype(other), Types> && ...) &&
                   (!std::same_as<Types, UTypes> && ...))))
        : TupleImpl<0, Types...>(std::move(other)) {
    }

    template <typename U1, typename U2>
    explicit(!LvalConvertibleDeduction<U1, U2>::value) Tuple(const std::pair<U1, U2>& pair)
        requires(sizeof...(Types) == 2 && CopyConstructibleDeduction<U1, U2>::value)
        : TupleImpl<0, Types...>(pair.first, pair.second) {
    }

    template <typename U1, typename U2>
    explicit(!RvalConvertibleDeduction<U1, U2>::value) Tuple(std::pair<U1, U2>&& pair)
        requires(sizeof...(Types) == 2 && MoveConstructibleDeduction<U1, U2>::value)
        : TupleImpl<0, Types...>(std::move(pair.first), std::move(pair.second)) {
    }

    Tuple(const Tuple& other)
        requires((std::is_copy_constructible_v<Types> && ...))
        : TupleImpl<0, Types...>(other) {
    }

    Tuple(Tuple&& other)
        requires((std::is_move_constructible_v<Types> && ...))
    = default;

    Tuple& operator=(const Tuple& other)
        requires((std::is_copy_assignable_v<Types> && ...))
    {
        if (this == &other) {
            return *this;
        }

        static_cast<TupleImpl<0, Types...>&>(*this) = other;
        return *this;
    }

    ~Tuple() {
    }

    Tuple& operator=(Tuple&& other) noexcept((std::is_nothrow_move_assignable_v<Types> && ...))
        requires((std::is_move_assignable_v<Types> && ...))
    {
        if (this == &other) {
            return *this;
        }

        static_cast<TupleImpl<0, Types...>&>(*this) = std::move(other);
        return *this;
    }

    // T& is used in "std::is_assignable<>" since T& is lvalue and T is rvalue that means
    // checking assignment U to temporary object T which has no sense
    template <typename... UTypes>
    Tuple& operator=(const Tuple<UTypes...>& other)
        requires(sizeof...(Types) == sizeof...(UTypes) &&
                 (std::is_assignable_v<Types&, const UTypes&> && ...))
    {
        static_cast<TupleImpl<0, Types...>&>(*this) = other;
        return *this;
    }

    template <typename... UTypes>
    Tuple& operator=(Tuple<UTypes...>&& other)
        requires(sizeof...(Types) == sizeof...(UTypes) &&
                 (std::is_assignable_v<Types&, UTypes> && ...))
    {
        static_cast<TupleImpl<0, Types...>&>(*this) = std::move(other);
        return *this;
    }

    template <typename E1, typename E2>
    Tuple& operator=(const std::pair<E1, E2>& pair)
        requires(IsPairCopyAssignable<E1, E2>::value)
    {
        Get<0>(*this) = pair.first;
        Get<1>(*this) = pair.second;
        return *this;
    }

    template <typename E1, typename E2>
    Tuple& operator=(std::pair<E1, E2>&& pair)
        requires(IsPairMoveAssignable<E1, E2>::value)
    {
        Get<0>(*this) = std::forward<E1>(pair.first);
        Get<1>(*this) = std::forward<E2>(pair.second);
        return *this;
    }

    // it was on cpp, let it be here
    // void Swap(Tuple& tuple) noexcept((std::is_nothrow_swappable_v<Types> && ...)) {
    //     static_cast<TupleImpl<0, Types...>&>(*this).Swap(tuple);
    // }

private:
};

// only definition since it is used only in "requires"
// "requires" accepts only expressions, it doesn't call the func, in fact
template <typename T>
void CheckImplicit(T);

// next traits are for MakeTuple
template <typename T>
struct UnwrapRefWrapper {
    using Type = T;
};

// overload for correct processing "std::ref", "std::cref" as a result of "std::decay"
template <typename T>
struct UnwrapRefWrapper<std::reference_wrapper<T>> {
    using Type = T&;
};

template <typename T>
using UnwrapDecayT = typename UnwrapRefWrapper<std::decay_t<T>>::Type;

template <typename... Types>
auto MakeTuple(Types&&... args) {
    return Tuple<UnwrapDecayT<Types>...>(std::forward<Types>(args)...);
}

template <typename... Args>
auto Tie(Args&... args) noexcept {
    return Tuple<Args&...>(args...);
}

// "Tuple<Types...>" is incorrect since this will create a copy in Tuple
template <typename... Types>
Tuple<Types&&...> ForwardAsTuple(Types&&... args) noexcept {
    return Tuple<Types&&...>(std::forward<Types>(args)...);
}

template <size_t Index, typename Head, typename... Tail>
struct TypeElement : TypeElement<Index - 1, Tail...> {};

template <typename Head, typename... Tail>
struct TypeElement<0, Head, Tail...> {
    using Type = Head;
};

// template <size_t Index, typename... Types>
// struct TupleElement;

// template <size_t Index, typename Head, typename... Tail>
// struct TupleElement<Index, Tuple<Head, Tail...>> : TupleElement<Index - 1, Tuple<Tail...>> {};

// template <typename Head, typename... Tail>
// struct TupleElement<0, Tuple<Head, Tail...>> {
//     using Type = Head;
// };

// static_cast with TupleImpl<Index, Types...> will unpack "Types..." naive
// So there won't be any deducted types
template <size_t Index, typename... Types>
typename TypeElement<Index, Types...>::Type& Get(Tuple<Types...>& tuple) noexcept {
    return GetImpl<Index>(tuple);
}

template <size_t Index, typename Head, typename... Tail>
Head& GetImpl(TupleImpl<Index, Head, Tail...>& tuple) noexcept {
    return tuple.value_;
}

template <size_t Index, typename... Types>
const typename TypeElement<Index, Types...>::Type& Get(const Tuple<Types...>& tuple) noexcept {
    return GetImpl<Index>(tuple);
}

template <size_t Index, typename Head, typename... Tail>
const Head& GetImpl(const TupleImpl<Index, Head, Tail...>& tuple) noexcept {
    return tuple.value_;
}

template <size_t Index, typename... Types>
typename TypeElement<Index, Types...>::Type&& Get(Tuple<Types...>&& tuple) noexcept {
    return GetImpl<Index>(std::move(tuple));
}

template <size_t Index, typename Head, typename... Tail>
Head&& GetImpl(TupleImpl<Index, Head, Tail...>&& tuple) noexcept {
    return std::move(tuple.value_);
}

// template <size_t Index, typename... Types>
// const typename TypeElement<Index, Types...>::Type&& Get(Tuple<Types...>&& tuple) noexcept {
//     return GetImpl<Index>(std::move(tuple));
// }

// template <size_t Index, typename Head, typename... Tail>
// const Head&& GetImpl(const TupleImpl<Index, Head, Tail...>&& tuple) noexcept {
//     return std::move(tuple.value_);
// }

// Created for Get<T> to check the uniqueness of T
// The first one is created in order to declare overload "template <typename T> struct Contains<T>"
template <typename, typename...>
struct Contains;

template <typename T, typename Head, typename... Tail>
struct Contains<T, Head, Tail...>
    : std::integral_constant<size_t, std::same_as<T, Head> + Contains<T, Tail...>::value> {};

template <typename T>
struct Contains<T> : std::integral_constant<size_t, 0> {};

// Created for Get<T> to check the index of T in Types
// There's no overload for failure of finding type since it will trigger CE as it demanded
template <typename T, typename Head, typename... Tails>
struct TypeIndex : std::integral_constant<size_t, 1 + TypeIndex<T, Tails...>::value> {};

template <typename T, typename... Tails>
struct TypeIndex<T, T, Tails...> : std::integral_constant<size_t, 0> {};

template <typename T, typename... Types>
T& Get(Tuple<Types...>& tuple) noexcept
    requires(Contains<T, Types...>::value == 1)
{
    return Get<TypeIndex<T, Types...>::value>(tuple);
}

template <typename T, typename... Types>
const T& Get(const Tuple<Types...>& tuple) noexcept
    requires(Contains<T, Types...>::value == 1)
{
    return Get<TypeIndex<T, Types...>::value>(tuple);
}

template <typename T, typename... Types>
T&& Get(Tuple<Types...>&& tuple) noexcept
    requires(Contains<T, Types...>::value == 1)
{
    return Get<TypeIndex<T, Types...>::value>(std::move(tuple));
}

template <typename T, typename... Types>
const T&& Get(const Tuple<Types...>&& tuple) noexcept
    requires(Contains<T, Types...>::value == 1)
{
    return Get<TypeIndex<T, Types...>::value>(std::move(tuple));
}

template <typename... TTypes, typename... UTypes, size_t... Is>
bool CheckEqualty(const Tuple<TTypes...>& left, const Tuple<UTypes...>& right,
                  std::index_sequence<Is...>) {
    return ((Get<Is>(left) == Get<Is>(right)) && ...);
}

template <typename... TTypes, typename... UTypes>
bool operator==(const Tuple<TTypes...>& left, const Tuple<UTypes...>& right) {
    return CheckEqualty(left, right, std::make_index_sequence<sizeof...(TTypes)>{});
}

template <typename ResultType, typename... TTypes, typename... UTypes, size_t... Is>
bool ThreeWayImpl(const Tuple<TTypes...>& left, const Tuple<UTypes...>& right,
                  std::index_sequence<Is...>) {
    ResultType result = ResultType::equivalent;
    ((result == ResultType::equivalent ? (result = (Get<Is>(left) <=> Get<Is>(right)))
                                       : ResultType{}),
     ...);

    return result;
}

template <typename... TTypes, typename... UTypes>
auto operator<=>(const Tuple<TTypes...>& left, const Tuple<UTypes...>& right) {
    using ResultType = std::common_comparison_category_t<decltype(std::declval<TTypes>() <=>
                                                                  std::declval<UTypes>())...>;

    return ThreeWayImpl<ResultType>(left, right, std::make_index_sequence<sizeof...(TTypes)>{});
}

// everything below is for TupleCat() realization
template <typename...>
struct GetTuple;

template <typename... Types>
struct GetTuple<Tuple<Types...>> {
    using Type = Tuple<Types...>;
};

template <typename T>
struct GetTuple<T> {
    using Type = Tuple<T>;
};

template <typename E1, typename E2>
struct GetTuple<std::pair<E1, E2>> {
    using Type = Tuple<E1, E2>;
};

template <typename T>
using GetTupleType = typename GetTuple<T>::Type;

template <typename...>
struct MergeTupleTypes;

template <typename... Types, typename... UTypes, typename... Tail>
struct MergeTupleTypes<Tuple<Types...>, Tuple<UTypes...>, Tail...>
    : MergeTupleTypes<typename MergeTupleTypes<Tuple<Types...>, Tuple<UTypes...>>::Type, Tail...> {
};

template <typename... Types, typename... UTypes>
struct MergeTupleTypes<Tuple<Types...>, Tuple<UTypes...>> {
    using Type = Tuple<Types..., UTypes...>;
};

template <typename...>
struct SizeOf;

template <typename... Types>
struct SizeOf<Tuple<Types...>> : std::integral_constant<size_t, sizeof...(Types)> {};

template <typename E1, typename E2>
struct SizeOf<std::pair<E1, E2>> : std::integral_constant<size_t, 2> {};

template <typename T>
struct SizeOf<T> : std::integral_constant<size_t, 1> {};

template <typename...>
struct TotalSize;

template <typename Head, typename... Tail>
struct TotalSize<Head, Tail...> : std::integral_constant<size_t, SizeOf<std::decay_t<Head>>::value +
                                                                     TotalSize<Tail...>::value> {};

template <typename Head>
struct TotalSize<Head> : std::integral_constant<size_t, SizeOf<std::decay_t<Head>>::value> {};

template <typename T>
struct IsTuple : std::false_type {};

template <typename... Types>
struct IsTuple<Tuple<Types...>> : std::true_type {};

template <typename T>
struct IsPair : std::false_type {};

template <typename E1, typename E2>
struct IsPair<std::pair<E1, E2>> : std::true_type {};

template <size_t Index, typename Head, typename... Tail>
decltype(auto) GetElement(Head&& first, Tail&&... args) {
    using DecayedHead = std::decay_t<Head>;
    if constexpr (Index > SizeOf<DecayedHead>::value - 1) {
        return GetElement<Index - SizeOf<DecayedHead>::value>(std::forward<Tail>(args)...);
    } else if constexpr (IsPair<DecayedHead>::value) {
        if (Index == 0) {
            return std::forward<decltype(first.first)>(first.first);
        } else {
            return std::forward<decltype(first.second)>(first.second);
        }
    } else if constexpr (IsTuple<DecayedHead>::value) {
        return Get<Index>(std::forward<Head>(first));
    } else {
        return std::forward<Head>(first);
    }
}

template <typename ResultType, typename... Tuples, size_t... Is>
ResultType TupleCatImpl(std::index_sequence<Is...>, Tuples&&... args) {
    return ResultType(GetElement<Is>(std::forward<Tuples>(args)...)...);
}

template <typename... Tuples>
MergeTupleTypes<GetTupleType<std::decay_t<Tuples>>...>::Type TupleCat(Tuples&&... args) {
    using ResultType = typename MergeTupleTypes<GetTupleType<std::decay_t<Tuples>>...>::Type;
    return TupleCatImpl<ResultType>(std::make_index_sequence<TotalSize<Tuples...>::value>{},
                                    std::forward<Tuples>(args)...);
}