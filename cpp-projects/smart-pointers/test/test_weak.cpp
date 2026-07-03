#include "smart_pointers.hpp"

#include <memory>
#include <random>
#include <tuple>
#include <vector>

#include <gtest/gtest.h>

struct Base {
    virtual ~Base() {
    }
};

struct Derived : public Base {};

template <int Tag>
struct TaggedDestructionCounter {
    inline static int destroyed = 0;

    static int GetTag() {
        return Tag;
    }

    ~TaggedDestructionCounter() {
        ++destroyed;
    }
};

std::pair<SharedPtr<std::vector<int>>, SharedPtr<std::vector<int>>> MakePtrs() {
    std::pair<SharedPtr<std::vector<int>>, SharedPtr<std::vector<int>>> ptrs;

    ptrs.first = SharedPtr<std::vector<int>>(new std::vector<int>(1'000'000));
    (*ptrs.first)[0] = 1;

    std::vector<int>& vec = *ptrs.first;
    ptrs.second = SharedPtr<std::vector<int>>(new std::vector<int>(vec));
    (*ptrs.second)[0] = 2;

    return ptrs;
}

struct Node;

struct Next {
    SharedPtr<Node> shared;
    WeakPtr<Node> weak;

    Next(const SharedPtr<Node>& shared) : shared(shared) {
    }

    Next(const WeakPtr<Node>& weak) : weak(weak) {
    }
};

struct Node {
    static int constructed;
    static int destructed;

    int value;
    Next next;

    Node(int value) : value(value), next(SharedPtr<Node>()) {
        ++constructed;
    }

    Node(int value, const SharedPtr<Node>& next) : value(value), next(next) {
        ++constructed;
    }

    Node(int value, const WeakPtr<Node>& next) : value(value), next(next) {
        ++constructed;
    }

    ~Node() {
        ++destructed;
    }
};

int Node::constructed = 0;
int Node::destructed = 0;

SharedPtr<Node> GetCyclePtr(int cycle_size) {
    SharedPtr<Node> head(new Node(0));
    SharedPtr<Node> prev(head);
    for (int i = 1; i < cycle_size; ++i) {
        SharedPtr<Node> current(new Node(i));
        prev->next.shared = current;
        prev = current;
    }
    prev->next.weak = head;
    return head;
}

TEST(WeakPtr, Expired) {
    auto sp = SharedPtr<int>(new int(23));
    WeakPtr<int> weak = sp;
    ASSERT_FALSE(weak.Expired());

    {
        auto shared = SharedPtr<int>(new int(42));
        weak = shared;
        ASSERT_EQ(weak.UseCount(), 1);
        ASSERT_FALSE(weak.Expired());
    }

    ASSERT_EQ(weak.UseCount(), 0);
    ASSERT_TRUE(weak.Expired());
}

TEST(WeakPtr, Interaction) {
    auto sp = SharedPtr<int>(new int(23));
    WeakPtr<int> weak = sp;

    auto wp = weak;
    ASSERT_EQ(weak.UseCount(), 1);
    ASSERT_EQ(wp.UseCount(), 1);

    auto wwp = std::move(weak);
    ASSERT_EQ(wwp.UseCount(), 1);
    ASSERT_EQ(weak.UseCount(), 0);
    ASSERT_TRUE(weak.Expired());

    auto ssp = wwp.Lock();
    ASSERT_EQ(sp.UseCount(), 2);
    ASSERT_EQ(ssp.Get(), sp.Get());

    sp = ssp;
    ssp = sp;
    ASSERT_EQ(ssp.UseCount(), 2);

    sp = std::move(ssp);
    ASSERT_EQ(sp.UseCount(), 1);

    ssp.Reset();  // nothing should happen
    sp.Reset();
}

TEST(WeakPtr, Stress) {
    unsigned int useless_value = 0;
    for (int i = 0; i < 100'000; ++i) {
        SharedPtr<Node> head = GetCyclePtr(8);
        SharedPtr<Node> next_head = head->next.shared;
        ASSERT_EQ(next_head.UseCount(), 2);
        useless_value += 19'937 * i * next_head.UseCount();

        head.Reset();
        ASSERT_EQ(next_head.UseCount(), 1);
    }
    std::ignore = useless_value;

    ASSERT_EQ(Node::constructed, 800'000);
    ASSERT_EQ(Node::destructed, 800'000);
}

TEST(WeakPtr, Inheritance) {
    SharedPtr<Derived> dsp(new Derived());

    SharedPtr<Base> bsp = dsp;

    WeakPtr<Derived> wdsp = dsp;
    WeakPtr<Base> wbsp = dsp;
    WeakPtr<Base> wwbsp = wdsp;

    ASSERT_EQ(dsp.UseCount(), 2);

    bsp = std::move(dsp);
    ASSERT_EQ(bsp.UseCount(), 1);

    bsp.Reset();
    ASSERT_TRUE(wdsp.Expired());
    ASSERT_TRUE(wbsp.Expired());
    ASSERT_TRUE(wwbsp.Expired());
}

TEST(WeakPtr, Constness) {
    SharedPtr<int> sp(new int(42));
    const WeakPtr<int> wp(sp);
    ASSERT_FALSE(wp.Expired());

    auto ssp = wp.Lock();
    ASSERT_NE(ssp.Get(), nullptr);
    ASSERT_EQ(ssp.Get(), sp.Get());
}

struct NeitherDefaultNorCopyConstructible {
    NeitherDefaultNorCopyConstructible() = delete;
    NeitherDefaultNorCopyConstructible(const NeitherDefaultNorCopyConstructible&) = delete;
    NeitherDefaultNorCopyConstructible& operator=(const NeitherDefaultNorCopyConstructible&) =
        delete;

    NeitherDefaultNorCopyConstructible(NeitherDefaultNorCopyConstructible&&) = default;
    NeitherDefaultNorCopyConstructible& operator=(NeitherDefaultNorCopyConstructible&&) = default;

    explicit NeitherDefaultNorCopyConstructible(int x) : x(x) {
    }

    int x;
};

struct Accountant {
    static int constructed;
    static int destructed;

    Accountant() {
        ++constructed;
    }

    Accountant(const Accountant&) {
        ++constructed;
    }

    ~Accountant() {
        ++destructed;
    }
};

int Accountant::constructed = 0;
int Accountant::destructed = 0;

int allocated = 0;
int deallocated = 0;

int allocate_called = 0;
int deallocate_called = 0;

int new_called = 0;
int delete_called = 0;

int construct_called = 0;
int destroy_called = 0;

struct VerySpecialType {};

void* operator new(size_t n, VerySpecialType) {
    return std::malloc(n);
}

void operator delete(void* ptr, VerySpecialType) {
    std::free(ptr);
}

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

template <typename T>
struct MyAllocator {
    using value_type = T;

    MyAllocator() = default;

    template <typename U>
    MyAllocator(const MyAllocator<U>&) {
    }

    T* allocate(size_t n) {
        ++allocate_called;
        allocated += n * sizeof(T);
        return static_cast<T*>(::operator new(n * sizeof(T), VerySpecialType()));
    }

    void deallocate(T* p, size_t n) {
        ++deallocate_called;
        deallocated += n * sizeof(T);
        ::operator delete(static_cast<void*>(p), VerySpecialType());
    }

    template <typename U, typename... Args>
    void construct(U* ptr, Args&&... args) {
        ++construct_called;
        ::new (static_cast<void*>(ptr)) U(std::forward<Args>(args)...);
    }

    template <typename U>
    void destroy(U* ptr) {
        ++destroy_called;
        ptr->~U();
    }
};

void InitCounters() {
    allocated = 0;
    deallocated = 0;
    allocate_called = 0;
    deallocate_called = 0;
    construct_called = 0;
    destroy_called = 0;
}

TEST(MakeShared, NeitherDefaultNorCopyConstructible) {
    Accountant::constructed = 0;
    Accountant::destructed = 0;

    InitCounters();

    new_called = 0;
    delete_called = 0;

    {
        auto sp =
            MakeShared<NeitherDefaultNorCopyConstructible>(NeitherDefaultNorCopyConstructible(0));
        WeakPtr<NeitherDefaultNorCopyConstructible> wp = sp;
        auto ssp = sp;
        sp.Reset();
        ASSERT_FALSE(wp.Expired());

        ssp.Reset();
        ASSERT_TRUE(wp.Expired());
    }

    ASSERT_EQ(new_called, 1);
    ASSERT_EQ(delete_called, 1);
}

TEST(MakeShared, Accountant) {
    Accountant::constructed = 0;
    Accountant::destructed = 0;

    InitCounters();

    new_called = 0;
    delete_called = 0;

    {
        auto sp = MakeShared<Accountant>();
        ASSERT_EQ(Accountant::constructed, 1);

        WeakPtr<Accountant> wp = sp;
        auto ssp = sp;
        sp.Reset();
        ASSERT_EQ(Accountant::constructed, 1);
        ASSERT_EQ(Accountant::destructed, 0);

        ASSERT_FALSE(wp.Expired());
        ssp.Reset();
        ASSERT_EQ(Accountant::destructed, 1);
    }

    ASSERT_EQ(new_called, 1);
    ASSERT_EQ(delete_called, 1);
}

TEST(AllocateShared, MoveOnly) {
    Accountant::constructed = 0;
    Accountant::destructed = 0;

    InitCounters();

    new_called = 0;
    delete_called = 0;

    {
        MyAllocator<NeitherDefaultNorCopyConstructible> alloc;
        auto sp = AllocateShared<NeitherDefaultNorCopyConstructible>(
            alloc, NeitherDefaultNorCopyConstructible(0));
        int count = allocated;
        ASSERT_GT(allocated, 0);
        ASSERT_EQ(allocate_called, 1);

        WeakPtr<NeitherDefaultNorCopyConstructible> wp = sp;
        auto ssp = sp;
        sp.Reset();
        ASSERT_EQ(count, allocated);
        ASSERT_EQ(deallocated, 0);

        ASSERT_FALSE(wp.Expired());
        ssp.Reset();
        ASSERT_EQ(count, allocated);
    }

    ASSERT_EQ(allocated, deallocated);

    ASSERT_EQ(allocate_called, 1);
    ASSERT_EQ(deallocate_called, 1);
    ASSERT_EQ(construct_called, 1);
    ASSERT_EQ(destroy_called, 1);
}

TEST(AllocateShared, Accountant) {
    Accountant::constructed = 0;
    Accountant::destructed = 0;

    InitCounters();

    new_called = 0;
    delete_called = 0;

    {
        MyAllocator<Accountant> alloc;
        auto sp = AllocateShared<Accountant>(alloc);
        int count = allocated;
        ASSERT_GT(allocated, 0);
        ASSERT_EQ(allocate_called, 1);
        ASSERT_EQ(Accountant::constructed, 1);

        WeakPtr<Accountant> wp = sp;
        auto ssp = sp;
        sp.Reset();
        ASSERT_EQ(count, allocated);
        ASSERT_EQ(deallocated, 0);
        ASSERT_EQ(Accountant::constructed, 1);
        ASSERT_EQ(Accountant::destructed, 0);

        ASSERT_FALSE(wp.Expired());
        ssp.Reset();
        ASSERT_EQ(count, allocated);
    }

    ASSERT_EQ(allocated, deallocated);

    ASSERT_EQ(Accountant::constructed, 1);
    ASSERT_EQ(Accountant::destructed, 1);

    ASSERT_EQ(allocate_called, 1);
    ASSERT_EQ(deallocate_called, 1);
    ASSERT_EQ(construct_called, 1);
    ASSERT_EQ(destroy_called, 1);

    ASSERT_EQ(new_called, 0);
    ASSERT_EQ(delete_called, 0);
}

int mother_created = 0;
int mother_destroyed = 0;
int son_created = 0;
int son_destroyed = 0;

struct Mother {
    Mother() {
        ++mother_created;
    }

    virtual ~Mother() {
        ++mother_destroyed;
    }
};

struct Son : public Mother {
    Son() {
        ++son_created;
    }

    virtual ~Son() {
        ++son_destroyed;
    }
};

TEST(SharedPtr, Inheritance) {
    mother_created = 0;
    mother_destroyed = 0;
    son_created = 0;
    son_destroyed = 0;

    InitCounters();

    {
        SharedPtr<Son> sp(new Son());

        SharedPtr<Mother> mp(new Mother());

        mp = sp;

        sp.Reset(new Son());
    }
    ASSERT_EQ(son_created, 2);
    ASSERT_EQ(son_destroyed, 2);
    ASSERT_EQ(mother_created, 3);
    ASSERT_EQ(mother_destroyed, 3);
}

TEST(SharedPtr, CustomAllocator) {
    mother_created = 0;
    mother_destroyed = 0;
    son_created = 0;
    son_destroyed = 0;

    InitCounters();

    {
        MyAllocator<Son> alloc;
        auto sp = AllocateShared<Son>(alloc);

        SharedPtr<Mother> mp = sp;

        sp.Reset(new Son());
    }
    ASSERT_EQ(son_created, 2);
    ASSERT_EQ(son_destroyed, 2);
    ASSERT_EQ(mother_created, 2);
    ASSERT_EQ(mother_destroyed, 2);

    ASSERT_EQ(allocated, deallocated);
    ASSERT_EQ(allocate_called, 1);
    ASSERT_EQ(deallocate_called, 1);
    ASSERT_EQ(construct_called, 1);
    ASSERT_EQ(destroy_called, 1);
}

int custom_deleter_called = 0;

struct MyDeleter {
    template <typename T>
    void operator()(T*) {
        ++custom_deleter_called;
    }
};

TEST(SharedPtr, CustomDeleter) {
    MyDeleter deleter;
    int x = 0;

    new_called = 0;
    delete_called = 0;

    {
        SharedPtr<int> sp(&x, deleter);

        auto ssp = std::move(sp);

        auto sssp = ssp;

        ssp = MakeShared<int>(5);
    }
    ASSERT_EQ(custom_deleter_called, 1);

    // 1 for ControlBlock in sp and 1 for makeShared
    ASSERT_EQ(new_called, 2);
    ASSERT_EQ(delete_called, 2);

    new_called = 0;
    delete_called = 0;

    InitCounters();

    custom_deleter_called = 0;

    Accountant::constructed = 0;
    Accountant::destructed = 0;

    Accountant acc;
    {
        MyAllocator<Accountant> alloc;
        MyDeleter deleter;

        SharedPtr<Accountant> sp(&acc, deleter, alloc);

        auto ssp = std::move(sp);

        auto sssp = ssp;

        ssp = MakeShared<Accountant>();
    }

    ASSERT_EQ(new_called, 1);  // for makeShared
    ASSERT_EQ(delete_called, 1);
    ASSERT_EQ(allocate_called, 1);
    ASSERT_EQ(deallocate_called, 1);
    ASSERT_EQ(allocated, deallocated);

    ASSERT_EQ(Accountant::constructed, 2);
    ASSERT_EQ(Accountant::destructed, 1);

    ASSERT_EQ(construct_called, 0);
    ASSERT_EQ(destroy_called, 0);
    ASSERT_EQ(custom_deleter_called, 1);
}

class PrivateObject {
public:
    int GetValue() const {
        return value_;
    }

private:
    PrivateObject(int v) : value_(v) {
    }

    ~PrivateObject() = default;

    int value_;

    template <typename U>
    friend struct MyAllocator;
};

TEST(AllocateShared, OnlyAllocatorIsFriend) {
    MyAllocator<PrivateObject> alloc;

    auto ptr = AllocateShared<PrivateObject>(alloc, 100);

    ASSERT_EQ(ptr->GetValue(), 100);
}

struct Payload {
    int value;
    inline static int active_count = 0;

    Payload(int v) : value(v) {
        ++active_count;
    }

    ~Payload() {
        --active_count;
    }
};

struct TwinPtr {
    SharedPtr<Payload> mine;
    std::shared_ptr<Payload> std_ptr;

    void Verify() const {
        if (mine.Get() == nullptr) {
            ASSERT_EQ(std_ptr.get(), nullptr);
        } else {
            ASSERT_NE(std_ptr.get(), nullptr);
            ASSERT_EQ(mine.UseCount(), static_cast<size_t>(std_ptr.use_count()));
            ASSERT_EQ(mine->value, std_ptr->value);
        }
    }
};

struct TwinWeak {
    WeakPtr<Payload> mine;
    std::weak_ptr<Payload> std_ptr;

    void Verify() const {
        ASSERT_EQ(mine.UseCount(), static_cast<size_t>(std_ptr.use_count()));
        ASSERT_EQ(mine.Expired(), std_ptr.expired());
    }
};

enum class FuzzAction {
    CreateShared,
    CopyShared,
    MoveShared,
    LockWeak,
    CreateWeak,
    ResetShared,
    SwapShared,
    EraseShared,
    Count,
};

void RunDeepDifferentialFuzz(int iterations, uint32_t seed) {
    std::mt19937 gen(seed);
    std::vector<TwinPtr> pool;
    std::vector<TwinWeak> weak_pool;

    std::uniform_int_distribution<int> action_dist(0, static_cast<int>(FuzzAction::Count) - 1);

    for (int i = 0; i < iterations; ++i) {
        auto action = static_cast<FuzzAction>(action_dist(gen));

        if (action == FuzzAction::CreateShared || pool.empty()) {
            auto val = std::make_shared<Payload>(i);
            pool.push_back({MakeShared<Payload>(i), val});
        } else if (action == FuzzAction::CopyShared) {
            size_t idx = gen() % pool.size();
            pool.push_back({pool[idx].mine, pool[idx].std_ptr});
        } else if (action == FuzzAction::MoveShared) {
            size_t idx = gen() % pool.size();
            TwinPtr moved;
            moved.mine = std::move(pool[idx].mine);
            moved.std_ptr = std::move(pool[idx].std_ptr);
            pool.push_back(std::move(moved));
        } else if (action == FuzzAction::LockWeak) {
            if (!weak_pool.empty()) {
                size_t idx = gen() % weak_pool.size();
                TwinPtr locked;
                locked.mine = weak_pool[idx].mine.Lock();
                locked.std_ptr = weak_pool[idx].std_ptr.lock();
                pool.push_back(std::move(locked));
            }
        } else if (action == FuzzAction::CreateWeak) {
            size_t idx = gen() % pool.size();
            weak_pool.push_back(
                {WeakPtr<Payload>(pool[idx].mine), std::weak_ptr<Payload>(pool[idx].std_ptr)});
        } else if (action == FuzzAction::ResetShared) {
            size_t idx = gen() % pool.size();
            pool[idx].mine.Reset();
            pool[idx].std_ptr.reset();
        } else if (action == FuzzAction::SwapShared) {
            if (pool.size() >= 2) {
                size_t i1 = gen() % pool.size();
                size_t i2 = gen() % pool.size();
                std::swap(pool[i1].mine, pool[i2].mine);
                std::swap(pool[i1].std_ptr, pool[i2].std_ptr);
            }
        } else if (action == FuzzAction::EraseShared) {
            size_t idx = gen() % pool.size();
            pool.erase(pool.begin() + idx);
        }

        if (!pool.empty()) {
            pool[gen() % pool.size()].Verify();
        }
        if (!weak_pool.empty()) {
            weak_pool[gen() % weak_pool.size()].Verify();
        }
    }

    pool.clear();
    weak_pool.clear();
    ASSERT_EQ(Payload::active_count, 0);
}

TEST(SharedPtr, DifferentialWithStd) {
    Payload::active_count = 0;
    RunDeepDifferentialFuzz(10'000, 0xDEADBEEF);
}
