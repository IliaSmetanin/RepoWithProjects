#include "pool_allocator.hpp"

#include "test/common.hpp"

#include <list>
#include <new>

#include <gtest/gtest.h>

TEST(PoolAllocator, Sanity) {
    using DataT = size_t;
    using Alloc = PoolAllocator<DataT, kBigSize>;

    PoolStorage<kBigSize> big_storage;
    std::list<DataT, Alloc> big_list{Alloc{big_storage}};
    for (size_t i = 0; i < kMediumSize; ++i) {
        big_list.push_back(i);
    }

    ASSERT_TRUE(std::equal(big_list.begin(), big_list.end(), IotaIterator<DataT>{DataT{0}}));
}

TEST(PoolAllocator, Alignment) {
    constexpr size_t kStorageSize = 5'000;

    PoolStorage<kStorageSize> storage;
    PoolAllocator<char, kStorageSize> charalloc(storage);
    PoolAllocator<int, kStorageSize> intalloc(charalloc);

    auto* pchar = charalloc.allocate(3);
    auto* pint = intalloc.allocate(1);

    ASSERT_NE((void*)pchar, (void*)pint);
    ASSERT_EQ(reinterpret_cast<uintptr_t>(pint) % alignof(int), 0);

    charalloc.deallocate(pchar, 3);

    pchar = charalloc.allocate(555);

    intalloc.deallocate(pint, 1);

    PoolAllocator<long double, kStorageSize> ldalloc(charalloc);

    auto* pld = ldalloc.allocate(25);

    ASSERT_EQ(reinterpret_cast<uintptr_t>(pld) % alignof(long double), 0);

    charalloc.deallocate(pchar, 555);
    ldalloc.deallocate(pld, 25);
}

TEST(PoolAllocator, OutOfMemory) {
    using DataT = size_t;
    constexpr size_t kBytesCount = kSmallSize * sizeof(DataT);
    using Alloc = PoolAllocator<DataT, kBytesCount>;

    PoolStorage<kBytesCount> small_storage;
    Alloc alloc{small_storage};

    std::array<DataT*, kSmallSize> data;
    for (size_t i = 0; i < kSmallSize; ++i) {
        ASSERT_THROW(alloc.allocate(kSmallSize + 1 - i), std::bad_alloc);
        data[i] = alloc.allocate(1);
    }

    ASSERT_THROW(alloc.allocate(1), std::bad_alloc);

    for (size_t i = 0; i < kSmallSize; ++i) {
        alloc.deallocate(data[i], 1);
    }
}

template <typename Tp, size_t sz, size_t other = 0>
void TestConcepts() {
    static_assert(!std::is_default_constructible<PoolAllocator<Tp, sz>>::value);
    static_assert(std::is_trivially_copy_constructible<PoolAllocator<Tp, sz>>::value);
    static_assert(std::is_same<Tp, typename PoolAllocator<Tp, sz>::value_type>::value);
    static_assert((!std::is_constructible<PoolAllocator<Tp, sz>, PoolAllocator<Tp, other>>::value) ^
                  (sz == other));
}

//         construct   no construct
// sz ==     True          False
// sz !=     False         True

template <typename Tp, size_t I, size_t... Js>
void TestConceptsInner(std::index_sequence<Js...>) {
    (TestConcepts<Tp, I, Js + 1>(), ...);
}

template <typename Tp, size_t... Is, size_t N>
void TestConceptsOuter(std::index_sequence<Is...>, std::integral_constant<size_t, N>) {
    (TestConceptsInner<Tp, Is + 1>(std::make_index_sequence<N>{}), ...);
}

TEST(PoolAllocator, Concepts) {
    constexpr size_t kCheckBound = 64;
    TestConceptsOuter<size_t>(std::make_index_sequence<kCheckBound>{},
                              std::integral_constant<size_t, kCheckBound>{});
}