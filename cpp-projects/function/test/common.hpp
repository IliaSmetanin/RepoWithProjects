#include <functional>
#include <numeric>
#include <vector>

#include <gtest/gtest.h>

int new_called = 0;
int delete_called = 0;

void* operator new(size_t n) {
    ++new_called;
    return std::malloc(n);
}

void operator delete(void* ptr) noexcept {
    ++delete_called;
    std::free(ptr);
}

void* operator new[](std::size_t count) {
    return operator new(count);
}

void* operator new(std::size_t count, std::align_val_t /*al*/) {
    return operator new(count);
}

void* operator new[](std::size_t count, std::align_val_t /*al*/) {
    return operator new(count);
}

void* operator new(std::size_t count, const std::nothrow_t& /*tag*/) noexcept {
    try {
        return operator new(count);
    } catch (...) {
        return nullptr;
    }
}

void* operator new[](std::size_t count, const std::nothrow_t& /*tag*/) noexcept {
    try {
        return operator new(count);
    } catch (...) {
        return nullptr;
    }
}

void* operator new(std::size_t count, std::align_val_t al, const std::nothrow_t& /*tag*/) noexcept {
    try {
        return operator new(count, al);
    } catch (...) {
        return nullptr;
    }
}

void* operator new[](std::size_t count, std::align_val_t al,
                     const std::nothrow_t& /*tag*/) noexcept {
    try {
        return operator new(count, al);
    } catch (...) {
        return nullptr;
    }
}

void operator delete(void* ptr, std::size_t /*count*/) noexcept {
    operator delete(ptr);
}

void operator delete[](void* ptr) noexcept {
    operator delete(ptr);
}

void operator delete[](void* ptr, std::size_t /*count*/) noexcept {
    operator delete(ptr);
}

void operator delete(void* ptr, std::align_val_t /*al*/) noexcept {
    operator delete(ptr);
}

void operator delete[](void* ptr, std::align_val_t /*al*/) noexcept {
    operator delete(ptr);
}

void operator delete(void* ptr, std::size_t /*count*/, std::align_val_t /*al*/) noexcept {
    operator delete(ptr);
}

void operator delete[](void* ptr, std::size_t /*count*/, std::align_val_t /*al*/) noexcept {
    operator delete(ptr);
}

void operator delete(void* ptr, const std::nothrow_t& /*tag*/) noexcept {
    operator delete(ptr);
}

void operator delete[](void* ptr, const std::nothrow_t& /*tag*/) noexcept {
    operator delete(ptr);
}

void operator delete(void* ptr, std::align_val_t /*al*/, const std::nothrow_t& /*tag*/) noexcept {
    operator delete(ptr);
}

void operator delete[](void* ptr, std::align_val_t /*al*/, const std::nothrow_t& /*tag*/) noexcept {
    operator delete(ptr);
}

struct Helper {
    int Method1(int z) {
        return x + z;
    }

    int Method2(int z) {
        return y + z;
    }

    int x = 3;
    int y = 30;
};

int Sum(int x, int y) {
    return x + y;
}

int Multiply(int x, int y) {
    return x * y;
}

template <typename R, typename T>
R TemplateTestFunction(T t) {
    return static_cast<R>(t);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestStaticAssertions() {
    struct TestStructConst {
        void ConstMethod(int) const {
        }

        void NonConstMethod(int) {
        }
    };

    static_assert(std::is_constructible_v<Func<void(TestStructConst&, int)>,
                                          decltype(&TestStructConst::ConstMethod)>);
    static_assert(!std::is_constructible_v<Func<void(const TestStructConst&, int)>,
                                           decltype(&TestStructConst::NonConstMethod)>);

    static_assert(std::is_assignable_v<Func<void(TestStructConst&, int)>,
                                       decltype(&TestStructConst::ConstMethod)>);
    static_assert(!std::is_assignable_v<Func<void(const TestStructConst&, int)>,
                                        decltype(&TestStructConst::NonConstMethod)>);

    static_assert(std::is_assignable_v<Func<void(TestStructConst&, int)>,
                                       Func<void(const TestStructConst&, int)>>);
    static_assert(!std::is_assignable_v<Func<void(const TestStructConst&, int)>,
                                        Func<void(TestStructConst&, int)>>);

    struct TestStructRvalue {
        void UsualMethod(int) {
        }

        void RvalueMethod(int) && {
        }

        void LvalueMethod(int) & {
        }

        void ConstLvalueMethod(int) const& {
        }
    };

    static_assert(std::is_constructible_v<Func<void(TestStructRvalue&&, int)>,
                                          decltype(&TestStructRvalue::UsualMethod)>);
    static_assert(std::is_constructible_v<Func<void(TestStructRvalue&&, int)>,
                                          decltype(&TestStructRvalue::RvalueMethod)>);
    static_assert(!std::is_constructible_v<Func<void(TestStructRvalue&&, int)>,
                                           decltype(&TestStructRvalue::LvalueMethod)>);
    static_assert(std::is_constructible_v<Func<void(TestStructRvalue&&, int)>,
                                          decltype(&TestStructRvalue::ConstLvalueMethod)>);

    static_assert(
        !std::is_invocable_v<Func<void(TestStructRvalue&&, int)>, TestStructRvalue&, int>);
    static_assert(
        std::is_invocable_v<Func<void(TestStructRvalue&&, int)>, TestStructRvalue&&, int>);
    static_assert(
        !std::is_invocable_v<Func<void(TestStructRvalue&, int)>, TestStructRvalue&&, int>);

    static_assert(std::is_assignable_v<Func<void(TestStructRvalue&&, int)>,
                                       decltype(&TestStructRvalue::UsualMethod)>);
    static_assert(std::is_assignable_v<Func<void(TestStructRvalue&&, int)>,
                                       decltype(&TestStructRvalue::RvalueMethod)>);
    static_assert(!std::is_assignable_v<Func<void(TestStructRvalue&&, int)>,
                                        decltype(&TestStructRvalue::LvalueMethod)>);
    static_assert(std::is_assignable_v<Func<void(TestStructRvalue&&, int)>,
                                       decltype(&TestStructRvalue::ConstLvalueMethod)>);

    static_assert(!std::is_assignable_v<Func<void(TestStructRvalue&&, int)>,
                                        Func<void(TestStructRvalue&, int)>>);
    static_assert(!std::is_assignable_v<Func<void(TestStructRvalue&, int)>,
                                        Func<void(TestStructRvalue&&, int)>>);
    static_assert(std::is_assignable_v<Func<void(TestStructRvalue&&, int)>,
                                       Func<void(const TestStructRvalue&, int)>>);
    static_assert(!std::is_assignable_v<Func<void(const TestStructRvalue&, int)>,
                                        Func<void(TestStructRvalue&&, int)>>);
}

struct AllocatorGuard {
    AllocatorGuard() {
        new_called = delete_called = 0;
    }

    ~AllocatorGuard() {
        EXPECT_EQ(new_called, 0);
        EXPECT_EQ(delete_called, 0);
    }
};

template <template <typename> typename Func, bool IsMoveOnly>
void TestWorks() {
    AllocatorGuard guard;
    Func<int(int, int)> func = Sum;
    ASSERT_EQ(func(5, 10), 15);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestTemplate() {
    AllocatorGuard guard;
    Func<int(double)> func = TemplateTestFunction<double, int>;
    ASSERT_EQ(func(123.456), 123);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestLambda() {
    AllocatorGuard guard;
    auto lambda = [](int a, int b, int c, int d) { return a * b + c * d; };

    Func<int(int, int, int, int)> func = lambda;
    ASSERT_EQ(func(2, 3, 20, 30), 606);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestPointer() {
    AllocatorGuard guard;
    auto func_ptr = +[]() { return 10; };
    Func<int()> func = func_ptr;
    ASSERT_EQ(func(), 10);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestFieldsAndMethods() {
    AllocatorGuard guard;
    Helper helper;

    Func<int(Helper&, int)> func = &Helper::Method1;
    ASSERT_EQ(func(helper, 10), 13);

    Func<int&(Helper&)> attr = &Helper::x;
    ASSERT_EQ(attr(helper), 3);

    attr(helper) = 10;
    ASSERT_EQ(func(helper, 10), 20);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestChangeSimpleObject() {
    AllocatorGuard guard;

    Func<int(int, int)> func = Sum;
    ASSERT_EQ(func(5, 10), 15);

    func = Multiply;
    ASSERT_EQ(func(5, 10), 50);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestAssignmentOperator() {
    AllocatorGuard guard;

    Func<int(int, int)> func_1 = Sum;
    Func<int(int, int)> func_2 = Multiply;

    ASSERT_EQ(func_1(5, 10), 15);

    if constexpr (!IsMoveOnly) {
        func_1 = func_2;
        ASSERT_EQ(func_1(5, 10), 50);
        ASSERT_EQ(func_2(5, 10), 50);
    }

    func_1 = std::move(func_2);
    ASSERT_EQ(func_1(5, 10), 50);
    ASSERT_FALSE(func_2);

    ASSERT_EQ(std::move(func_1)(5, 10), 50);
    ASSERT_EQ(func_1(5, 10), 50);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestChangeObject() {
    AllocatorGuard guard;

    Helper helper;
    Func<int(Helper&, int)> func = &Helper::Method1;
    Func<int&(Helper&)> attr = &Helper::x;

    ASSERT_EQ(func(helper, 10), 13);

    attr(helper) = 10;
    ASSERT_EQ(func(helper, 10), 20);

    func = &Helper::Method2;
    ASSERT_EQ(func(helper, 10), 40);

    attr = &Helper::y;
    ASSERT_EQ(attr(helper), 30);

    attr(helper) = 100;
    ASSERT_EQ(func(helper, 10), 110);

    int x = 25;
    auto lambda = [&x](Helper&) -> int& { return x; };
    attr = lambda;

    attr(helper) = 55;
    ASSERT_EQ(x, 55);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestEmpty() {
    AllocatorGuard guard;
    Func<int(int, int)> func;

    EXPECT_EQ(func, nullptr);
    EXPECT_FALSE(func);
    EXPECT_THROW(func(1, 2), std::bad_function_call);

    func = Multiply;
    EXPECT_NE(func, nullptr);
    EXPECT_TRUE(func);

    auto f = std::move(func);
    EXPECT_FALSE(func);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestInvoke() {
    Func<int(int, int)> func = Sum;
    EXPECT_EQ(std::invoke(func, 5, 10), 15);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestSmallObjectOptimization() {
    AllocatorGuard guard;

    if constexpr (!IsMoveOnly) {
        Func func = Sum;
        ASSERT_EQ(func(5, 10), 15);
    }

    if constexpr (!IsMoveOnly) {
        Func func = [](int x) { return x * x * x; };
        ASSERT_EQ(func(10), 1000);
    }

    {
        Func<int(int)> func = [&](int x) { return x == 0 ? 1 : x * func(x - 1); };
        ASSERT_EQ(func(5), 120);
    }
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestCallChangesState() {
    std::vector<int> vec = {1, 2, 3};

    Func<int()> func = [&]() { return std::accumulate(vec.begin(), vec.end(), 0); };

    EXPECT_EQ(func(), 6);

    vec.push_back(4);
    EXPECT_EQ(func(), 10);

    Func<int()> func2 = std::move(func);
    EXPECT_EQ(func2(), 10);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestBind() {
    using std::placeholders::_1;
    using std::placeholders::_2;

    auto lambda = [](int a, int b, int c, int d) { return a * b + c * d; };

    Func<int(int, int)> func = std::bind(lambda, 2, _2, _1, 30);
    ASSERT_EQ(func(20, 3), 606);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestAssignEmpty() {
    AllocatorGuard guard;

    Func<int(int, int)> func = Sum;
    Func<int(int, int)> empty;

    if constexpr (!IsMoveOnly) {
        func = empty;
        ASSERT_FALSE(func);
        EXPECT_THROW(func(1, 2), std::bad_function_call);

        func = Sum;
    }

    func = std::move(empty);
    ASSERT_FALSE(func);
    EXPECT_THROW(func(1, 2), std::bad_function_call);

    func = Multiply;
    ASSERT_EQ(func(5, 10), 50);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestMoveAssignmentWithState() {
    AllocatorGuard guard;

    struct Counter {
        int operator()() {
            return ++value;
        }

        int value;
    };

    Func<int()> src = Counter{10};
    Func<int()> dst = Counter{100};

    ASSERT_EQ(dst(), 101);
    ASSERT_EQ(src(), 11);

    dst = std::move(src);

    ASSERT_FALSE(src);
    ASSERT_EQ(dst(), 12);
    ASSERT_EQ(dst(), 13);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestReferenceWrapperAssignment() {
    AllocatorGuard guard;

    struct Counter {
        Counter() = default;
        Counter(const Counter&) = delete;
        Counter& operator=(const Counter&) = delete;

        int operator()(int x) {
            value += x;
            return value;
        }

        int value = 0;
    };

    Counter counter;
    Func<int(int)> func = [](int x) { return -x; };

    func = std::ref(counter);

    ASSERT_EQ(func(5), 5);
    ASSERT_EQ(counter.value, 5);

    counter.value = 10;
    ASSERT_EQ(func(7), 17);
}

struct Small {
    int operator()() {
        return ++value;
    }

    int value;
};

template <template <typename> typename Func, bool IsMoveOnly>
void TestSmallObjectBasic() {
    AllocatorGuard guard;

    ASSERT_LE(sizeof(Small), 16);

    Func<int()> func = Small{1};
    ASSERT_EQ(func(), 2);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestAssignSmallObjectNoAllocation() {
    AllocatorGuard guard;

    ASSERT_LE(sizeof(Small), 16);

    Func<int()> func = Small{1};
    ASSERT_EQ(func(), 2);

    func = Small{2};
    ASSERT_EQ(func(), 3);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestTargetTypeEmpty() {
    Func<int(int)> func;
    EXPECT_EQ(func.TargetType(), typeid(void));
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestTargetTypeFunctionPointer() {
    Func<int(int, int)> func = Sum;
    EXPECT_EQ(func.TargetType(), typeid(int (*)(int, int)));
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestTargetTypeLambda() {
    auto lambda = [](int x) { return x + 1; };
    Func<int(int)> func = lambda;
    EXPECT_EQ(func.TargetType(), typeid(lambda));
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestTargetTypeFunctor() {
    Func<int()> func = Small{0};
    EXPECT_EQ(func.TargetType(), typeid(Small));
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestTargetTypeChangesOnAssign() {
    Func<int(int, int)> func = Sum;
    EXPECT_EQ(func.TargetType(), typeid(int (*)(int, int)));

    auto lambda = [](int a, int b) { return a - b; };
    func = lambda;
    EXPECT_EQ(func.TargetType(), typeid(lambda));
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestTargetReturnsStoredFunctionPointer() {
    using FnPtr = int (*)(int, int);
    Func<int(int, int)> func = Sum;

    auto* ptr = func.template Target<FnPtr>();
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(*ptr, &Sum);
    EXPECT_EQ((*ptr)(3, 4), 7);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestTargetReturnsStoredFunctor() {
    Func<int()> func = Small{10};
    auto* ptr = func.template Target<Small>();
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(ptr->value, 10);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestTargetReturnsStoredLambda() {
    auto lambda = [v = 5](int x) { return x + v; };
    Func<int(int)> func = lambda;

    auto* ptr = func.template Target<decltype(lambda)>();
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ((*ptr)(2), 7);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestTargetForMemberPointer() {
    using MemPtr = int (Helper::*)(int);
    Func<int(Helper&, int)> func = &Helper::Method1;

    auto* ptr = func.template Target<MemPtr>();
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(*ptr, &Helper::Method1);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestTargetForReferenceWrapper() {
    Small s{42};
    Func<int()> func = std::ref(s);

    EXPECT_EQ(func.TargetType(), typeid(std::reference_wrapper<Small>));

    auto* wrapped = func.template Target<std::reference_wrapper<Small>>();
    ASSERT_NE(wrapped, nullptr);
    EXPECT_EQ(&wrapped->get(), &s);

    auto* raw = func.template Target<Small>();
    EXPECT_EQ(raw, nullptr);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestTargetWrongTypeReturnsNull() {
    Func<int(int, int)> func = Sum;

    EXPECT_EQ((func.template Target<int (*)(double, double)>()), nullptr);
    EXPECT_EQ((func.template Target<int>()), nullptr);
    EXPECT_EQ((func.template Target<Small>()), nullptr);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestTargetEmptyReturnsNull() {
    Func<int(int)> func;

    EXPECT_EQ((func.template Target<int (*)(int)>()), nullptr);
    EXPECT_EQ((func.template Target<Small>()), nullptr);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestTargetAllowsMutation() {
    struct Counter {
        int value;

        int operator()() {
            return ++value;
        }
    };

    Func<int()> func = Counter{0};

    auto* ptr = func.template Target<Counter>();
    ASSERT_NE(ptr, nullptr);

    EXPECT_EQ(func(), 1);
    EXPECT_EQ(ptr->value, 1);

    ptr->value = 100;
    EXPECT_EQ(func(), 101);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestTargetConstReturnsConstPointer() {
    using FnPtr = int (*)(int, int);

    Func<int(int, int)> func = Sum;
    const auto& cref = func;

    using NonConstResult = decltype(func.template Target<FnPtr>());
    using ConstResult = decltype(cref.template Target<FnPtr>());

    static_assert(std::is_same_v<NonConstResult, FnPtr*>);
    static_assert(std::is_same_v<ConstResult, const FnPtr*>);

    auto* ptr = cref.template Target<FnPtr>();
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(*ptr, &Sum);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestTargetCopyIsIndependent() {
    if constexpr (!IsMoveOnly) {
        Func<int()> a = Small{0};
        Func<int()> b = a;

        auto* pa = a.template Target<Small>();
        auto* pb = b.template Target<Small>();

        ASSERT_NE(pa, nullptr);
        ASSERT_NE(pb, nullptr);
        EXPECT_NE(pa, pb);

        a();
        a();
        EXPECT_EQ(pa->value, 2);
        EXPECT_EQ(pb->value, 0);
    }
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestTargetAfterReassignment() {
    using FnPtr = int (*)(int, int);

    Func<int(int, int)> func = Sum;
    EXPECT_EQ(*func.template Target<FnPtr>(), &Sum);

    func = Multiply;
    auto* ptr = func.template Target<FnPtr>();
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(*ptr, &Multiply);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestTargetAfterMove() {
    Func<int()> src = Small{7};
    Func<int()> dst = std::move(src);

    EXPECT_EQ(src.TargetType(), typeid(void));
    EXPECT_EQ((src.template Target<Small>()), nullptr);

    EXPECT_EQ(dst.TargetType(), typeid(Small));
    auto* ptr = dst.template Target<Small>();
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(ptr->value, 7);
}

template <template <typename> typename Func, bool IsMoveOnly>
void TestTargetForLargeFunctor() {
    struct Large {
        int payload[16];

        int operator()() const {
            return payload[0];
        }
    };

    static_assert(sizeof(Large) > 16);

    Large large{};
    large.payload[0] = 123;

    Func<int()> func = large;

    EXPECT_EQ(func.TargetType(), typeid(Large));

    auto* ptr = func.template Target<Large>();
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(ptr->payload[0], 123);
    EXPECT_NE(ptr, &large);

    EXPECT_EQ((func.template Target<int>()), nullptr);
}
