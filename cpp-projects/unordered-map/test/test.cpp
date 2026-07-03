#include "unordered_map.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <functional>
#include <iterator>
#include <memory>
#include <mutex>
#include <numeric>
#include <random>
#include <ranges>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include <gtest/gtest.h>

// template <typename Key, typename Value, typename Hash = std::hash<Key>,
//           typename EqualTo = std::equal_to<Key>,
//           typename Alloc = std::allocator<std::pair<const Key, Value>>>
// using UnorderedMap = std::unordered_map<Key, Value, Hash, EqualTo, Alloc>;

constexpr size_t operator""_sz(unsigned long long int x) {
    return static_cast<size_t>(x);
}

namespace basic {

// Just a simple SFINAE trick to check CE presence when it's necessary
// Stay tuned, we'll discuss this kind of tricks in our next lectures ;)
template <typename T>
decltype(UnorderedMap<T, T>().cbegin()->second = 0, int()) TestConstIteratorDoesntAllowModification(
    T) {
    assert(false);
}

template <typename... FakeArgs>
void TestConstIteratorDoesntAllowModification(FakeArgs...) {
}

struct VerySpecialType {
    int x = 0;

    explicit VerySpecialType(int x) : x(x) {
    }

    VerySpecialType(const VerySpecialType&) = delete;
    VerySpecialType& operator=(const VerySpecialType&) = delete;

    VerySpecialType(VerySpecialType&&) = default;
    VerySpecialType& operator=(VerySpecialType&&) = default;
};

struct NeitherDefaultNorCopyConstructible {
    VerySpecialType x;

    NeitherDefaultNorCopyConstructible() = delete;
    NeitherDefaultNorCopyConstructible(const NeitherDefaultNorCopyConstructible&) = delete;
    NeitherDefaultNorCopyConstructible& operator=(const NeitherDefaultNorCopyConstructible&) =
        delete;

    NeitherDefaultNorCopyConstructible(VerySpecialType&& x) : x(std::move(x)) {
    }

    NeitherDefaultNorCopyConstructible(NeitherDefaultNorCopyConstructible&&) = default;
    NeitherDefaultNorCopyConstructible& operator=(NeitherDefaultNorCopyConstructible&&) = default;

    bool operator==(const NeitherDefaultNorCopyConstructible& other) const {
        return x.x == other.x.x;
    }
};

}  // namespace basic

namespace std {
template <>
struct hash<basic::NeitherDefaultNorCopyConstructible> {
    size_t operator()(const basic::NeitherDefaultNorCopyConstructible& x) const {
        return hash<int>()(x.x.x);
    }
};
}  // namespace std

namespace basic {

template <typename T>
struct MyHash {
    size_t operator()(const T& p) const {
        return std::hash<int>()(p.second / p.first);
    }
};

template <typename T>
struct MyEqual {
    bool operator()(const T& x, const T& y) const {
        return y.second / y.first == x.second / x.first;
    }
};

struct OneMoreStrangeStruct {
    int first;
    int second;
};

bool operator==(const OneMoreStrangeStruct&, const OneMoreStrangeStruct&) = delete;

TEST(UnorderedMap, Simple) {
    UnorderedMap<std::string, int> m;

    m["aaaaa"] = 5;
    m["bbb"] = 6;
    m.At("bbb") = 7;
    ASSERT_EQ(m.Size(), 2_sz);

    ASSERT_EQ(m["aaaaa"], 5);
    ASSERT_EQ(m["bbb"], 7);
    ASSERT_EQ(m["ccc"], 0);

    ASSERT_EQ(m.Size(), 3_sz);

    ASSERT_THROW(m.At("xxxxxxxx"), std::out_of_range);
    auto it = m.Find("dddd");
    ASSERT_EQ(it, m.end());

    it = m.Find("bbb");
    ASSERT_EQ(it->second, 7);
    ++it->second;
    ASSERT_EQ(it->second, 8);

    for (auto& item : m) {
        --item.second;
    }
    ASSERT_EQ(m.At("aaaaa"), 4);

    {
        auto mm = m;
        m = std::move(mm);
    }

    auto res = m.Emplace("abcde", 2);
    ASSERT_TRUE(res.second);
}

TEST(UnorderedMap, BasicIterators) {
    UnorderedMap<double, std::string> m;

    std::vector<double> keys = {0.4, 0.3, -8.32, 7.5, 10.0, 0.0};
    std::vector<std::string> values = {
        "Summer has come and passed",     "The innocent can never last",
        "Wake me up when September ends", "Like my fathers come to pass",
        "Seven years has gone so fast",   "Wake me up when September ends",
    };

    m.Reserve(1'000'000);

    for (int i = 0; i < 6; ++i) {
        m.Insert({keys[i], values[i]});
    }

    auto beg = m.cbegin();
    std::string s = beg->second;
    auto it = m.begin();
    ++it;
    m.Erase(it++);
    it = m.begin();
    m.Erase(++it);

    ASSERT_EQ(beg->second, s);
    ASSERT_EQ(m.Size(), 4_sz);

    UnorderedMap<double, std::string> mm;
    std::vector<std::pair<const double, std::string>> elements = {
        {3.0, values[0]}, {5.0, values[1]}, {-10.0, values[2]}, {35.7, values[3]}};
    mm.Insert(elements.cbegin(), elements.cend());
    s = mm.begin()->second;

    m.Insert(mm.begin(), mm.end());
    ASSERT_EQ(mm.Size(), 4_sz);
    ASSERT_EQ(mm.begin()->second, s);

    m.Reserve(1'000'000);
    ASSERT_EQ(m.Size(), 8_sz);
    for (int i = 0; i < 10000; ++i) {
        int64_t h = 0;
        for (auto it = m.cbegin(); it != m.cend(); ++it) {
            h += static_cast<int>(it->first) + static_cast<int>((it->second)[0]);
        }
        std::ignore = h;
    }

    it = m.begin();
    ++it;
    s = it->second;
    for (double d = 100.0; d < 10100.0; d += 0.1) {
        m.Emplace(d, "a");
    }
    ASSERT_EQ(it->second, s);

    auto dist = std::distance(it, m.end());
    auto sz = m.Size();
    m.Erase(it, m.end());
    ASSERT_EQ(sz - dist, m.Size());

    for (double d = 200.0; d < 10200.0; d += 0.35) {
        auto it = m.Find(d);
        if (it != m.end()) {
            m.Erase(it);
        }
    }
}

TEST(UnorderedMap, NoRedundantCopies) {
    UnorderedMap<NeitherDefaultNorCopyConstructible, NeitherDefaultNorCopyConstructible> m;
    m.Reserve(10);
    m.Emplace(VerySpecialType(0), VerySpecialType(0));
    m.Reserve(1'000'000);
    std::pair<NeitherDefaultNorCopyConstructible, NeitherDefaultNorCopyConstructible> p{
        VerySpecialType(1), VerySpecialType(1)};

    m.Insert(std::move(p));

    ASSERT_EQ(m.Size(), 2_sz);

    m.At(VerySpecialType(1)) = VerySpecialType(0);

    {
        auto mm = std::move(m);
        m = std::move(mm);
    }
    m.Erase(m.begin());
    m.Erase(m.begin());
    ASSERT_EQ(m.Size(), 0_sz);
}

TEST(UnorderedMap, CustomHashAndCompare) {
    UnorderedMap<std::pair<int, int>, char, MyHash<std::pair<int, int>>,
                 MyEqual<std::pair<int, int>>>
        m;

    m.Insert({{1, 2}, 0});
    m.Insert({{2, 4}, 1});
    ASSERT_EQ(m.Size(), 1_sz);

    m[{3, 6}] = 3;
    ASSERT_EQ(m.At({4, 8}), 3);

    UnorderedMap<OneMoreStrangeStruct, int, MyHash<OneMoreStrangeStruct>,
                 MyEqual<OneMoreStrangeStruct>>
        mm;
    mm[{1, 2}] = 3;
    ASSERT_EQ(mm.At({5, 10}), 3);

    mm.Emplace(OneMoreStrangeStruct{3, 9}, 2);
    ASSERT_EQ(mm.Size(), 2_sz);
    mm.Reserve(1'000);
    mm.Erase(mm.begin());
    mm.Erase(mm.begin());
    for (int k = 1; k < 100; ++k) {
        for (int i = 1; i < 10; ++i) {
            mm.Insert({{i, k * i}, 0});
        }
    }
    std::string ans;
    std::string myans;
    for (auto it = mm.cbegin(); it != mm.cend(); ++it) {
        ans += std::to_string(it->second);
        myans += '0';
    }
    ASSERT_EQ(ans, myans);
}

// Finally, some tricky fixtures to test custom allocator.
// Done by professional, don't try to repeat
class Chaste {
private:
    int x_ = 0;
    Chaste() = default;

    Chaste(int x) : x_(x) {
    }

    // Nobody can construct me except this guy
    template <typename T>
    friend struct TheChosenOne;

public:
    Chaste(const Chaste&) = default;
    Chaste(Chaste&&) = default;

    bool operator==(const Chaste& other) const {
        return x_ == other.x_;
    }
};

}  // namespace basic

namespace std {
template <>
struct hash<basic::Chaste> {
    size_t operator()(const basic::Chaste& x) const noexcept {
        return std::hash<int>()(reinterpret_cast<const int&>(x));
    }
};
}  // namespace std

namespace basic {

template <typename T>
struct TheChosenOne : public std::allocator<T> {
    TheChosenOne() {
    }

    template <typename U>
    TheChosenOne(const TheChosenOne<U>&) {
    }

    template <typename... Args>
    void construct(T* p, Args&&... args) const {
        new (p) T(std::forward<Args>(args)...);
    }

    void construct(std::pair<const Chaste, Chaste>* p, int a, int b) const {
        new (p) std::pair<const Chaste, Chaste>(Chaste(a), Chaste(b));
    }

    void destroy(T* p) const {
        p->~T();
    }

    template <typename U>
    struct rebind {
        using other = TheChosenOne<U>;
    };
};

TEST(UnorderedMap, Allocator) {
    // This container mustn't construct or destroy any objects without using
    // TheChosenOne allocator
    UnorderedMap<Chaste, Chaste, std::hash<Chaste>, std::equal_to<Chaste>,
                 TheChosenOne<std::pair<const Chaste, Chaste>>>
        m;

    m.Emplace(0, 0);

    {
        auto mm = m;
        mm.Reserve(1'000);
        mm.Erase(mm.begin());
    }

    for (int i = 0; i < 1'000'000; ++i) {
        m.Emplace(i, i);
    }

    for (int i = 0; i < 500'000; ++i) {
        auto it = m.begin();
        ++it, ++it;
        m.Erase(m.begin(), it);
    }
}
}  // namespace basic

namespace additional {

struct Data {
    int data;

    auto operator<=>(const Data&) const = default;
};

struct Trivial : Data {};

constexpr Trivial operator""_tr(unsigned long long int x) {
    return Trivial{static_cast<int>(x)};
}

struct NonTrivial : Data {
    NonTrivial() = default;

    NonTrivial(int x) {
        data = x;
    }
};

NonTrivial operator""_ntr(unsigned long long int x) {
    return NonTrivial{static_cast<int>(x)};
}

struct NotDefaultConstructible {
    int data;

    NotDefaultConstructible() = delete;

    NotDefaultConstructible(int input) : data(input) {
    }

    auto operator<=>(const NotDefaultConstructible&) const = default;
};

struct NeitherDefaultNorCopyConstructible {
    int data;
    const bool moved = false;

    NeitherDefaultNorCopyConstructible() = delete;
    NeitherDefaultNorCopyConstructible(const NeitherDefaultNorCopyConstructible&) = delete;

    NeitherDefaultNorCopyConstructible(NeitherDefaultNorCopyConstructible&& other)
        : data(other.data), moved(true) {
    }

    NeitherDefaultNorCopyConstructible(int input) : data(input), moved(false) {
    }

    auto operator<=>(const NeitherDefaultNorCopyConstructible&) const = default;
};

}  // namespace additional

namespace std {
template <>
struct hash<additional::NeitherDefaultNorCopyConstructible> {
    size_t operator()(const additional::NeitherDefaultNorCopyConstructible& x) const noexcept {
        return std::hash<int>()(x.data);
    }
};
}  // namespace std

namespace additional {

template <typename T>
struct NotPropagatedCountingAllocator {
    size_t allocates_counter = 0;
    size_t deallocates_counter = 0;

    using propagate_on_container_move_assignment = std::false_type;
    using value_type = T;

    NotPropagatedCountingAllocator() = default;

    template <typename U>
    NotPropagatedCountingAllocator(const NotPropagatedCountingAllocator<U>& other)
        : allocates_counter(other.allocates_counter),
          deallocates_counter(other.deallocates_counter) {
    }

    template <typename U>
    NotPropagatedCountingAllocator(NotPropagatedCountingAllocator<U>&& other)
        : allocates_counter(other.allocates_counter),
          deallocates_counter(other.deallocates_counter) {
        other.allocates_counter = 0;
        other.deallocates_counter = 0;
    }

    bool operator==(const NotPropagatedCountingAllocator<T>&) const {
        return false;
    }

    T* allocate(size_t n) {
        ++allocates_counter;
        return std::allocator<T>().allocate(n);
    }

    void deallocate(T* pointer, size_t n) {
        ++deallocates_counter;
        return std::allocator<T>().deallocate(pointer, n);
    }
};

struct AllocatorStatistics {
    int allocations = 0;
    int constructs = 0;
    int destroys = 0;
    int deallocations = 0;
};

template <typename T>
struct AllCountingAllocator : std::allocator<T> {
    using Base = std::allocator<T>;

    template <typename U>
    struct rebind {
        using other = AllCountingAllocator<U>;
    };

    AllCountingAllocator() = default;

    AllCountingAllocator(const AllCountingAllocator& other) : Base(other), stats(other.stats) {
    }

    template <typename U>
    AllCountingAllocator(const AllCountingAllocator<U>& other) : Base(other), stats(other.stats) {
    }

    AllCountingAllocator& operator=(const AllCountingAllocator& other) {
        Base::operator=(other);
        stats = other.stats;
        return *this;
    }

    bool operator==(const AllCountingAllocator& other) const = default;

    T* allocate(size_t n) {
        ++stats.allocations;
        return Base::allocate(n);
    }

    template <typename... Args>
    void construct(T* ptr, Args&&... args) {
        std::construct_at(ptr, args...);
        ++stats.constructs;
    }

    void destroy(T* ptr) {
        ++stats.destroys;
        std::destroy_at(ptr);
    }

    void deallocate(T* pointer, size_t n) {
        ++stats.deallocations;
        Base::deallocate(pointer, n);
    }

    AllocatorStatistics stats;
};

template <typename T>
void AssignToSelf(T& value) {
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
    value = value;
#ifdef __clang__
#pragma clang diagnostic pop
#endif
}

const size_t kSmallSize = 17;
const size_t kMediumSize = 100;
const size_t kBigSize = 10'000;

namespace ranges = std::ranges;

template <typename Value>
auto MakeSmallMap() {
    UnorderedMap<int, Value> map;
    for (int i = 0; i < static_cast<int>(kSmallSize); ++i) {
        map.Emplace(i, Value{i});
    }
    return map;
}

bool MapsEqual(const auto& left, const auto& right) {
    return ranges::all_of(left,
                          [&right](const auto& pr) { return right.At(pr.first) == pr.second; });
}

TEST(UnorderedMap, IteratorConcepts) {
    using ContainerType = UnorderedMap<int, int>;
    using iterator = ContainerType::iterator;
    using const_iterator = ContainerType::const_iterator;

    // See https://en.cppreference.com/w/cpp/iterator/input_iterator for more information
    // Also check type aliases and methods from
    // https://en.cppreference.com/w/cpp/named_req/InputIterator
    static_assert(std::input_iterator<iterator>,
                  "Map iterator must satisfy concept 'input_iterator'");
    static_assert(std::input_iterator<const_iterator>,
                  "Map const_iterator must satisfy concept 'input_iterator'");

    // See https://en.cppreference.com/w/cpp/iterator/forward_iterator for more information
    // Also check type aliases and methods from
    // https://en.cppreference.com/w/cpp/named_req/ForwardIterator
    static_assert(std::forward_iterator<iterator>,
                  "Map iterator must satisfy concept 'forward_iterator'");
    static_assert(std::forward_iterator<const_iterator>,
                  "Map const_iterator must satisfy concept 'forward_iterator'");
}

TEST(UnorderedMap, DefaultConstructor) {
    UnorderedMap<int, int> defaulted;
    ASSERT_EQ(defaulted.Size(), 0_sz);
    UnorderedMap<int, NotDefaultConstructible> without_default;
    ASSERT_EQ(without_default.Size(), 0_sz);
}

TEST(UnorderedMap, CopyMoveConstructor) {
    {
        auto map = MakeSmallMap<Trivial>();
        auto copy = map;
        ASSERT_TRUE(MapsEqual(copy, map));
        auto move_copy = std::move(map);
        ASSERT_TRUE(MapsEqual(copy, move_copy));
        ASSERT_EQ(map.Size(), 0_sz);
        ASSERT_TRUE(map.Empty());
    }
    {
        auto map = MakeSmallMap<NonTrivial>();
        auto copy = map;
        ASSERT_TRUE(MapsEqual(copy, map));
        auto move_copy = std::move(map);
        ASSERT_TRUE(MapsEqual(copy, move_copy));
        ASSERT_EQ(map.Size(), 0_sz);
        ASSERT_TRUE(map.Empty());
    }
}

TEST(UnorderedMap, Assignment) {
    auto map = MakeSmallMap<Trivial>();
    ASSERT_TRUE(map.Size() == kSmallSize);
    ASSERT_FALSE(map.Empty());

    UnorderedMap<int, Trivial> map2;
    ASSERT_EQ(map2.Size(), 0_sz);
    ASSERT_TRUE(map2.Empty());

    map2 = map;
    ASSERT_TRUE(MapsEqual(map, map2));
    map2 = std::move(map);
    ASSERT_EQ(map.Size(), 0_sz);
    ASSERT_EQ(map2.Size(), kSmallSize);
}

TEST(UnorderedMap, MultipleSelfAssignment) {
    UnorderedMap<int, Trivial> map{kBigSize};
    constexpr size_t kIterCount = 10'000'000;
    for (size_t _ = 0; _ < kIterCount; ++_) {
        AssignToSelf(map);
    }
}

TEST(UnorderedMap, Swap) {
    auto map = MakeSmallMap<Trivial>();
    decltype(map) another;
    auto it = map.Find(1);
    auto address = &(*it);
    ASSERT_EQ(it->second, 1_tr);
    map.Swap(another);
    ASSERT_EQ(it->second, 1_tr);
    ASSERT_EQ(address->second, 1_tr);
}

TEST(UnorderedMap, Emplace) {
    UnorderedMap<int, NonTrivial> map;
    auto [place, did_insert] = map.Emplace(1, 1_ntr);
    ASSERT_EQ(map.At(1), 1_ntr);
    ASSERT_EQ(place, map.begin());
    ASSERT_TRUE(did_insert);

    auto [new_place, new_did_insert] = map.Emplace(2, 2_ntr);
    // update place as it could be invalidated by rehash
    place = map.Find(1);
    ASSERT_NE(place, new_place);
    ASSERT_TRUE(new_did_insert);
    ASSERT_EQ(map.At(2), 2_ntr);
    ASSERT_EQ(map.At(1), 1_ntr);

    auto [old_place, reinsert] = map.Emplace(1, 3_ntr);
    ASSERT_FALSE(reinsert);
    ASSERT_EQ(old_place, place);
    ASSERT_EQ(map.At(1), 1_ntr);
    ASSERT_EQ(map.At(2), 2_ntr);
}

TEST(UnorderedMap, EmplaceMove) {
    UnorderedMap<std::string, std::string> moving_map;
    std::string a = "a";
    std::string b = "b";
    std::string c = "c";

    moving_map.Emplace(a, a);
    ASSERT_EQ(a, "a");

    moving_map.Emplace(std::move(b), a);
    ASSERT_EQ(a, "a");
    ASSERT_TRUE(b.empty());

    moving_map.Emplace(std::move(c), std::move(a));
    ASSERT_TRUE(a.empty());
    ASSERT_TRUE(c.empty());
    ASSERT_EQ(moving_map.Size(), 3_sz);
    ASSERT_EQ(moving_map.At("a"), "a");
    ASSERT_EQ(moving_map.At("b"), "a");
    ASSERT_EQ(moving_map.At("c"), "a");
}

TEST(UnorderedMap, EmplaceAllocations) {
    UnorderedMap<int, int, std::hash<int>, std::equal_to<int>, AllCountingAllocator<int>> map;
    map.Reserve(3);

    auto old_alloc_state = map.GetAllocator();

    map.Emplace(1, 1);

    auto alloc_state = map.GetAllocator();

    ASSERT_EQ(alloc_state.stats.allocations - old_alloc_state.stats.allocations, 1);
}

TEST(UnorderedMap, Insert) {
    UnorderedMap<int, NonTrivial> map;

    auto [place, did_insert] = map.Insert({1, 1_ntr});
    ASSERT_EQ(map.At(1), 1_ntr);
    ASSERT_EQ(place, map.begin());
    ASSERT_TRUE(did_insert);

    auto [new_place, new_did_insert] = map.Insert({2, 2_ntr});
    place = map.Find(1);
    ASSERT_NE(place, new_place);
    ASSERT_TRUE(new_did_insert);
    ASSERT_EQ(map.At(2), 2_ntr);
    ASSERT_EQ(map.At(1), 1_ntr);

    auto [old_place, reinsert] = map.Insert({1, 3_ntr});
    ASSERT_FALSE(reinsert);
    ASSERT_EQ(old_place, place);
    ASSERT_EQ(map.At(1), 1_ntr);
    ASSERT_EQ(map.At(2), 2_ntr);
}

TEST(UnorderedMap, InsertMove) {
    UnorderedMap<std::string, std::string> moving_map;
    using Node = std::pair<std::string, std::string>;

    Node a{"a", "a"};
    Node b{"b", "b"};

    moving_map.Insert(a);
    ASSERT_EQ(a.first, "a");
    ASSERT_EQ(moving_map.Size(), 1_sz);

    moving_map.Insert(std::move(b));
    ASSERT_TRUE(b.first.empty());
    ASSERT_EQ(moving_map.Size(), 2_sz);

    ASSERT_EQ(moving_map.At("a"), "a");
    ASSERT_EQ(moving_map.At("b"), "b");
}

TEST(UnorderedMap, InsertRange) {
    UnorderedMap<int, NonTrivial> map;
    std::vector<std::pair<int, NonTrivial>> range;
    for (int i = 0; i < static_cast<int>(kMediumSize); ++i) {
        range.emplace_back(i, NonTrivial{i});
    }
    map.Insert(range.begin(), range.end());
    std::vector<int> indices(kSmallSize);
    std::iota(indices.begin(), indices.end(), 0);
    ASSERT_TRUE(std::all_of(indices.begin(), indices.end(),
                            [&](int item) { return map.At(item) == NonTrivial{item}; }));
}

TEST(UnorderedMap, InsertAllocations) {
    UnorderedMap<int, int, std::hash<int>, std::equal_to<int>, AllCountingAllocator<int>> map;
    map.Reserve(3);

    auto old_alloc_state = map.GetAllocator();

    map.Insert(std::make_pair(1, 1));
    map.Insert(std::make_pair<const int, int>(2, 1));

    auto alloc_state = map.GetAllocator();

    ASSERT_EQ(alloc_state.stats.allocations - old_alloc_state.stats.allocations, 2);
}

TEST(UnorderedMap, MoveInsertRange) {
    UnorderedMap<int, std::string> map;
    std::vector<int> indices(kSmallSize);
    std::iota(indices.begin(), indices.end(), 0);
    std::vector<std::pair<int, std::string>> storage;
    std::transform(indices.begin(), indices.end(), std::back_inserter(storage), [](int idx) {
        return std::pair<const int, std::string>{idx, std::to_string(idx)};
    });
    map.Insert(storage.begin(), storage.end());
    ASSERT_TRUE(
        ranges::all_of(storage, [](auto& pr) { return std::to_string(pr.first) == pr.second; }));

    map = UnorderedMap<int, std::string>();
    map.Insert(std::move_iterator(storage.begin()), std::move_iterator(storage.end()));
    ASSERT_EQ(storage.size(), kSmallSize);
    ASSERT_TRUE(ranges::all_of(storage, [&](auto& p) {
        bool equal = p.second.empty();
        EXPECT_TRUE(equal);
        return equal;
    }));
}

TEST(UnorderedMap, AtSubscript) {
    std::vector<int> indices(kSmallSize);
    std::iota(indices.begin(), indices.end(), 0);
    std::vector<std::pair<int, std::string>> range;
    std::transform(indices.begin(), indices.end(), std::back_inserter(range), [](int idx) {
        return std::pair<const int, std::string>{idx, std::to_string(idx)};
    });
    std::iota(indices.begin(), indices.end(), 0);

    UnorderedMap<int, std::string> map;
    map.Insert(range.begin(), range.end());
    for (int idx : indices) {
        ASSERT_EQ(std::to_string(idx), map.At(idx));
        ASSERT_EQ(std::to_string(idx), map[idx]);
    }

    ASSERT_THROW(map.At(-1) = "abacaba", std::out_of_range);
    map[-1] = "abacaba";
    ASSERT_EQ(map.At(-1), "abacaba");
    map.At(-1) = "qwerty";
    ASSERT_EQ(map[-1], "qwerty");
}

TEST(UnorderedMap, MoveSubscript) {
    std::vector<int> indices(kSmallSize);
    std::iota(indices.begin(), indices.end(), 0);
    std::vector<std::string> storage;
    std::transform(indices.begin(), indices.end(), std::back_inserter(storage),
                   [](int idx) { return std::to_string(idx); });

    UnorderedMap<std::string, std::string> map;
    map[std::move(storage[0])] = std::move(storage[1]);
    ASSERT_TRUE(storage[0].empty());
    ASSERT_TRUE(storage[1].empty());
    map[std::move(storage[2])] = storage[3];
    ASSERT_TRUE(storage[2].empty());
    ASSERT_EQ(storage[3], "3");
    map[storage[3]] = std::move(storage[4]);
    ASSERT_EQ(storage[3], "3");
    ASSERT_TRUE(storage[4].empty());
}

TEST(UnorderedMap, Find) {
    auto map = MakeSmallMap<Trivial>();
    auto existing = map.Find(1);
    ASSERT_EQ(existing->second, 1_tr);
    ASSERT_TRUE(map.Contains(1));

    auto non_existing = map.Find(-1);
    ASSERT_EQ(non_existing, map.end());
    ASSERT_FALSE(map.Contains(-1));
}

TEST(UnorderedMap, BucketBorders) {
    UnorderedMap<int, int, decltype([](int element) -> size_t {
                     return static_cast<size_t>(abs(element) % 10);
                 })>
        map;
    map.Emplace(1, 1);
    map.Emplace(11, 11);
    ASSERT_NE(map.Find(1), map.end());
    ASSERT_NE(map.Find(11), map.end());
    ASSERT_NE(map.Find(11), map.Find(1));
}

TEST(UnorderedMap, LoadFactor) {
    auto map = MakeSmallMap<Trivial>();
    auto max_val = std::max_element(map.begin(), map.end(), [](auto& left, auto& right) {
                       return left.first < right.first;
                   })->first;
    ASSERT_GT(map.LoadFactor(), 0.0f);
    auto new_load_factor = map.LoadFactor() / 2.0f;
    map.MaxLoadFactor(new_load_factor);
    for (auto i = max_val + 1; i < max_val + 1 + static_cast<int>(kMediumSize); ++i) {
        auto [_, inserted] = map.Emplace(i, Trivial{i});
        ASSERT_TRUE(inserted);
        ASSERT_GT(map.LoadFactor(), 0.0f);
        ASSERT_LE(map.LoadFactor(), new_load_factor);
    }
}

template <typename T>
struct Hash {
    size_t operator()(const T& key) const {
        return key % 2;
    }
};

TEST(UnorderedMap, BadHash) {
    UnorderedMap<int, int, Hash<int>> map;
    map.Emplace(1, 2);
    map.Emplace(2, 2);
    map.Emplace(3, 2);
    map.Emplace(4, 2);

    for (int i = 1; i < 4; ++i) {
        auto it = map.Find(i);
        ASSERT_EQ(it->first, i);
        ASSERT_TRUE(map.Contains(i));
    }
}

struct HashException : std::runtime_error {
    using std::runtime_error::runtime_error;
};

template <typename T>
struct ExceptionalHash {
    size_t operator()(const T&) const {
        throw HashException{"Hash exception"};
    }
};

TEST(UnorderedMap, HashWithException) {
    UnorderedMap<int, int, ExceptionalHash<int>, std::equal_to<int>,
                 AllCountingAllocator<std::pair<const int, int>>>
        map;
    ASSERT_THROW(map.Emplace(1, 1), HashException);

    auto alloc = map.GetAllocator();
    ASSERT_EQ(alloc.stats.constructs, alloc.stats.destroys);
}

template <typename T>
struct CountingHash {
    size_t operator()(const T& value) const {
        ++call_count;
        return value;
    }

    inline static size_t call_count = 0;
};

TEST(UnorderedMap, HashRehash) {
    constexpr size_t kElementCount = 50'000;
    UnorderedMap<int, int, CountingHash<int>> map;

    for (size_t i = 0; i < kElementCount; ++i) {
        map.Emplace(i, 0);
    }
    ASSERT_EQ(CountingHash<int>::call_count, kElementCount);

    map.Reserve(500'000);

    // Hash must not be called in Reserve
    ASSERT_EQ(CountingHash<int>::call_count, kElementCount);
}

struct ExplosiveException : public std::exception {};

struct Explosive {
    struct Safeguard {};

    inline static bool exploded = false;

    Explosive() : should_explode_(true) {
        throw ExplosiveException{};
    }

    Explosive(Safeguard) : should_explode_(false) {
    }

    Explosive(const Explosive&) : should_explode_(true) {
        throw ExplosiveException{};
    }

    Explosive(Explosive&&) : should_explode_(true) {
        throw ExplosiveException{};
    }

    Explosive& operator=(const Explosive&) {
        return *this;
    }

    Explosive& operator=(Explosive&&) {
        return *this;
    }

    bool operator==(const Explosive& other) const {
        if (this == &other) {
            return true;
        }
        return false;
    }

    ~Explosive() {
        exploded |= should_explode_;
    }

private:
    const bool should_explode_;
};

struct CountedException : public std::exception {};

template <int WhenThrow>
struct Counted {
    inline static int counter = 0;

    Counted() {
        ++counter;
        if (counter == WhenThrow) {
            --counter;
            throw CountedException();
        }
    }

    Counted(const Counted&) : Counted() {
    }

    Counted(const Counted&&) : Counted() {
    }

    bool operator==(const Counted& other) const {
        if (this == &other) {
            return true;
        }
        return false;
    }

    ~Counted() {
        --counter;
    }
};

template <typename T>
struct CountingHashInt {
    size_t operator()(const T&) const {
        ++call_count;
        return call_count / 2;
    }

    inline static size_t call_count = 0;
};

TEST(List, Exceptions) {
    UnorderedMap<Counted<1>, Counted<3>, CountingHashInt<Counted<1>>> near_zero{};
    ASSERT_THROW(near_zero[Counted<1>()], CountedException);
    ASSERT_EQ(Counted<1>::counter, 0);

    UnorderedMap<Explosive, Explosive, CountingHashInt<Explosive>> near_the_void(2);
    ASSERT_THROW(near_the_void.Emplace(), ExplosiveException);
    // See below.
    ASSERT_EQ(Explosive::exploded, false);

    {
        UnorderedMap<Explosive, Explosive, CountingHashInt<Explosive>> potentially_explosive_map(2);
    }
    // See below.
    ASSERT_EQ(Explosive::exploded, false);

    {
        UnorderedMap<Explosive, Explosive, CountingHashInt<Explosive>> guarded;
        auto safe = Explosive{Explosive::Safeguard{}};
        ASSERT_THROW(guarded.Emplace(safe, safe), ExplosiveException);
    }

    // Destructor should not be called for an object with not finished constructor. The only
    // destructor called is for explosive with the safeguard.
    ASSERT_EQ(Explosive::exploded, false);
}

struct VerySpecialType {
    int x = 0;

    explicit VerySpecialType(int x) : x(x) {
    }

    VerySpecialType(const VerySpecialType&) = delete;
    VerySpecialType& operator=(const VerySpecialType&) = delete;

    VerySpecialType(VerySpecialType&&) = default;
    VerySpecialType& operator=(VerySpecialType&&) = default;
};

struct NeitherDefaultNorCopyNorMoveConstructible {
    VerySpecialType x;

    NeitherDefaultNorCopyNorMoveConstructible() = delete;
    NeitherDefaultNorCopyNorMoveConstructible(const NeitherDefaultNorCopyNorMoveConstructible&) =
        delete;
    NeitherDefaultNorCopyNorMoveConstructible& operator=(
        const NeitherDefaultNorCopyNorMoveConstructible&) = delete;
    NeitherDefaultNorCopyNorMoveConstructible(NeitherDefaultNorCopyNorMoveConstructible&&) = delete;
    NeitherDefaultNorCopyNorMoveConstructible& operator=(
        NeitherDefaultNorCopyNorMoveConstructible&&) = delete;

    NeitherDefaultNorCopyNorMoveConstructible(VerySpecialType&& x) : x(std::move(x)) {
    }

    bool operator==(const NeitherDefaultNorCopyNorMoveConstructible& other) const {
        return x.x == other.x.x;
    }
};

}  // namespace additional

namespace std {
template <>
struct hash<additional::NeitherDefaultNorCopyNorMoveConstructible> {
    size_t operator()(const additional::NeitherDefaultNorCopyNorMoveConstructible& x) const {
        return hash<int>()(x.x.x);
    }
};
}  // namespace std

namespace additional {

// std::mutex is neither copyable nor movable. not copy-assignable
// https://en.cppreference.com/w/cpp/thread/mutex.html
// https://en.cppreference.com/w/cpp/utility/piecewise_construct.html
TEST(UnorderedMap, MutexMappedType) {
    UnorderedMap<int, std::mutex> map;
    ASSERT_TRUE(map.Empty());
    ASSERT_EQ(map.Size(), 0_sz);

    map.MaxLoadFactor(0.5f);
    map.Reserve(64);
    ASSERT_EQ(map.Size(), 0_sz);

    auto [it1, inserted1] =
        map.Emplace(std::piecewise_construct, std::forward_as_tuple(1), std::forward_as_tuple());

    ASSERT_TRUE(inserted1);
    ASSERT_EQ(it1->first, 1);

    auto [same_it1, inserted1_again] =
        map.Emplace(std::piecewise_construct, std::forward_as_tuple(1), std::forward_as_tuple());
    ASSERT_FALSE(inserted1_again);
    ASSERT_EQ(same_it1->first, 1);

    auto [it2, inserted2] =
        map.Emplace(std::piecewise_construct, std::forward_as_tuple(2), std::forward_as_tuple());
    auto [it3, inserted3] =
        map.Emplace(std::piecewise_construct, std::forward_as_tuple(3), std::forward_as_tuple());

    ASSERT_TRUE(inserted2);
    ASSERT_TRUE(inserted3);
    ASSERT_EQ(map.Size(), 3_sz);
    ASSERT_FALSE(map.Empty());

    {
        std::unique_lock<std::mutex> lock(map.At(1), std::try_to_lock);
        ASSERT_TRUE(lock.owns_lock());
    }
    {
        std::unique_lock<std::mutex> lock(map.At(2), std::try_to_lock);
        ASSERT_TRUE(lock.owns_lock());
    }

    ASSERT_TRUE(map.Contains(1));
    ASSERT_TRUE(map.Contains(2));
    ASSERT_TRUE(map.Contains(3));
    ASSERT_FALSE(map.Contains(100));

    ASSERT_NE(map.Find(2), map.end());
    ASSERT_EQ(map.Find(100), map.end());
    ASSERT_THROW(map.At(100), std::out_of_range);

    size_t iterated = 0;
    for (auto it = map.begin(); it != map.end(); ++it) {
        ++iterated;
    }
    ASSERT_EQ(iterated, map.Size());

    const auto& cmap = map;
    ASSERT_EQ(static_cast<size_t>(std::distance(cmap.cbegin(), cmap.cend())), map.Size());
    ASSERT_NE(cmap.Find(1), cmap.cend());

    auto* mutex1 = &map.At(1);
    auto* mutex2 = &map.At(2);

    map.Rehash(256);
    map.Reserve(512);

    ASSERT_EQ(&map.At(1), mutex1);
    ASSERT_EQ(&map.At(2), mutex2);
    ASSERT_GT(map.LoadFactor(), 0.0f);
    ASSERT_LE(map.LoadFactor(), 0.5f);

    map.Erase(map.Find(2));
    ASSERT_FALSE(map.Contains(2));
    ASSERT_EQ(map.Size(), 2_sz);

    auto it = map.Find(3);
    auto next = it;
    ++next;
    map.Erase(it, next);
    ASSERT_FALSE(map.Contains(3));
    ASSERT_EQ(map.Size(), 1_sz);

    UnorderedMap<int, std::mutex> other;
    other.Emplace(std::piecewise_construct, std::forward_as_tuple(10), std::forward_as_tuple());

    auto keep = map.Find(1);
    auto* keep_mutex = &keep->second;

    map.Swap(other);

    ASSERT_TRUE(map.Contains(10));
    ASSERT_TRUE(other.Contains(1));
    ASSERT_EQ(&other.At(1), keep_mutex);
    ASSERT_EQ(&keep->second, keep_mutex);

    auto moved = std::move(other);
    ASSERT_TRUE(other.Empty());
    ASSERT_TRUE(moved.Contains(1));

    UnorderedMap<int, std::mutex> assigned;
    assigned = std::move(moved);
    ASSERT_TRUE(moved.Empty());
    ASSERT_TRUE(assigned.Contains(1));
}

TEST(UnorderedMap, NeitherDefaultNorCopyNorMoveConstructible) {
    using T = NeitherDefaultNorCopyNorMoveConstructible;

    UnorderedMap<T, T> m;
    ASSERT_TRUE(m.Empty());
    ASSERT_EQ(m.Size(), 0_sz);

    m.MaxLoadFactor(0.5f);
    m.Reserve(10);

    auto [it0, inserted0] = m.Emplace(VerySpecialType(0), VerySpecialType(0));
    ASSERT_TRUE(inserted0);
    ASSERT_EQ(it0->first.x.x, 0);
    ASSERT_EQ(it0->second.x.x, 0);

    auto [same_it0, inserted0_again] = m.Emplace(VerySpecialType(0), VerySpecialType(10));
    ASSERT_FALSE(inserted0_again);
    ASSERT_EQ(same_it0->first.x.x, 0);
    ASSERT_EQ(same_it0->second.x.x, 0);

    auto [it1, inserted1] = m.Emplace(VerySpecialType(1), VerySpecialType(1));
    ASSERT_TRUE(inserted1);
    ASSERT_EQ(it1->first.x.x, 1);
    ASSERT_EQ(it1->second.x.x, 1);

    auto [it2, inserted2] =
        m.Emplace(std::piecewise_construct, std::forward_as_tuple(VerySpecialType(2)),
                  std::forward_as_tuple(VerySpecialType(2)));
    ASSERT_TRUE(inserted2);
    ASSERT_EQ(it2->first.x.x, 2);
    ASSERT_EQ(it2->second.x.x, 2);

    ASSERT_FALSE(m.Empty());
    ASSERT_EQ(m.Size(), 3_sz);

    ASSERT_TRUE(m.Contains(VerySpecialType(0)));
    ASSERT_TRUE(m.Contains(VerySpecialType(1)));
    ASSERT_TRUE(m.Contains(VerySpecialType(2)));
    ASSERT_FALSE(m.Contains(VerySpecialType(100)));

    ASSERT_EQ(m.At(VerySpecialType(1)).x.x, 1);
    ASSERT_THROW(m.At(VerySpecialType(100)), std::out_of_range);

    auto found = m.Find(VerySpecialType(2));
    ASSERT_NE(found, m.end());
    ASSERT_EQ(found->first.x.x, 2);
    ASSERT_EQ(found->second.x.x, 2);

    size_t iterated = 0;
    for (auto it = m.begin(); it != m.end(); ++it) {
        ++iterated;
    }
    ASSERT_EQ(iterated, m.Size());

    const auto& cm = m;
    ASSERT_EQ(static_cast<size_t>(std::distance(cm.cbegin(), cm.cend())), m.Size());
    ASSERT_NE(cm.Find(VerySpecialType(0)), cm.cend());

    auto* saved_node = &*m.Find(VerySpecialType(0));

    m.Reserve(1'000'000);
    m.Rehash(10'000'000);

    ASSERT_EQ(&*m.Find(VerySpecialType(0)), saved_node);
    ASSERT_LE(m.LoadFactor(), 0.5f);

    UnorderedMap<T, T> other;
    other.Emplace(VerySpecialType(100), VerySpecialType(100));

    auto keep = m.Find(VerySpecialType(0));
    auto* keep_ptr = &*keep;

    m.Swap(other);

    ASSERT_EQ(keep->first.x.x, 0);
    ASSERT_EQ(keep_ptr->second.x.x, 0);
    ASSERT_TRUE(other.Contains(VerySpecialType(0)));
    ASSERT_TRUE(other.Contains(VerySpecialType(1)));
    ASSERT_TRUE(other.Contains(VerySpecialType(2)));
    ASSERT_TRUE(m.Contains(VerySpecialType(100)));

    auto moved = std::move(m);
    ASSERT_TRUE(m.Empty());
    ASSERT_TRUE(moved.Contains(VerySpecialType(100)));

    UnorderedMap<T, T> assigned;
    assigned = std::move(other);
    ASSERT_TRUE(other.Empty());
    ASSERT_TRUE(assigned.Contains(VerySpecialType(0)));
    ASSERT_TRUE(assigned.Contains(VerySpecialType(1)));
    ASSERT_TRUE(assigned.Contains(VerySpecialType(2)));

    assigned.Erase(assigned.Find(VerySpecialType(1)));
    ASSERT_FALSE(assigned.Contains(VerySpecialType(1)));
    ASSERT_EQ(assigned.Size(), 2_sz);

    assigned.Erase(assigned.begin(), assigned.end());
    ASSERT_TRUE(assigned.Empty());

    moved.Erase(moved.begin());
    ASSERT_TRUE(moved.Empty());
}

template <typename T, bool PropagateOnCopy, bool PropagateOnMove, bool PropagateOnSwap,
          bool AlwaysEqual = true>
struct PropagatableCountingAllocator : std::allocator<T> {
    using Base = std::allocator<T>;

    using propagate_on_container_copy_assignment = std::bool_constant<PropagateOnCopy>;
    using propagate_on_container_move_assignment = std::bool_constant<PropagateOnMove>;
    using propagate_on_container_swap = std::bool_constant<PropagateOnSwap>;
    using is_always_equal = std::bool_constant<AlwaysEqual>;

    template <typename U>
    struct rebind {
        using other = PropagatableCountingAllocator<U, PropagateOnCopy, PropagateOnMove,
                                                    PropagateOnSwap, AlwaysEqual>;
    };

    PropagatableCountingAllocator() = default;

    explicit PropagatableCountingAllocator(int value) : Base(), value_(value) {
    }

    PropagatableCountingAllocator(const PropagatableCountingAllocator& other)
        : Base(other),
          value_(other.value_),
          allocations_(other.allocations_),
          deallocations_(other.deallocations_) {
    }

    template <typename U, bool P1, bool P2, bool P3, bool P4>
    PropagatableCountingAllocator(const PropagatableCountingAllocator<U, P1, P2, P3, P4>& other)
        : Base(other),
          value_(other.value_),
          allocations_(other.allocations_),
          deallocations_(other.deallocations_) {
    }

    PropagatableCountingAllocator& operator=(const PropagatableCountingAllocator& other) {
        Base::operator=(other);
        value_ = other.value_;
        allocations_ = other.allocations_;
        deallocations_ = other.deallocations_;
        return *this;
    }

    bool operator==(const PropagatableCountingAllocator& other) const {
        return AlwaysEqual || value_ == other.value_;
    }

    bool operator!=(const PropagatableCountingAllocator& other) const {
        return !(*this == other);
    }

    T* allocate(size_t n) {
        ++allocations_;
        return Base::allocate(n);
    }

    void deallocate(T* ptr, size_t n) {
        ++deallocations_;
        Base::deallocate(ptr, n);
    }

    int value_ = 0;
    int allocations_ = 0;
    int deallocations_ = 0;
};

template <class Map>
void AssertMapContains(const Map& map, std::initializer_list<std::pair<int, int>> expected) {
    ASSERT_EQ(map.Size(), expected.size());
    for (const auto& [k, v] : expected) {
        auto it = map.Find(k);
        ASSERT_NE(it, map.end());
        ASSERT_EQ(it->second, v);
    }
}

TEST(UnorderedMap, CopyAssignmentAlloc) {
    {
        using Alloc =
            PropagatableCountingAllocator<std::pair<const int, int>, true, false, true, true>;
        UnorderedMap<int, int, std::hash<int>, std::equal_to<int>, Alloc> map1(0, Alloc{1});
        map1.Insert({1, 10});
        ASSERT_GT(map1.GetAllocator().allocations_, 0);

        UnorderedMap<int, int, std::hash<int>, std::equal_to<int>, Alloc> map2(0, Alloc{2});
        map2 = map1;

        AssertMapContains(map2, {{1, 10}});
        ASSERT_EQ(map2.GetAllocator().value_, 1);
        ASSERT_GT(map2.GetAllocator().allocations_, 0);
    }

    {
        using Alloc =
            PropagatableCountingAllocator<std::pair<const int, int>, false, false, true, true>;
        UnorderedMap<int, int, std::hash<int>, std::equal_to<int>, Alloc> map1(0, Alloc{1});
        map1.Insert({1, 10});
        ASSERT_GT(map1.GetAllocator().allocations_, 0);

        UnorderedMap<int, int, std::hash<int>, std::equal_to<int>, Alloc> map2(0, Alloc{2});
        map2 = map1;

        AssertMapContains(map2, {{1, 10}});
        ASSERT_EQ(map2.GetAllocator().value_, 2);
        ASSERT_GT(map2.GetAllocator().allocations_, 0);
    }

    {
        using Alloc =
            PropagatableCountingAllocator<std::pair<const int, int>, false, false, true, false>;
        UnorderedMap<int, int, std::hash<int>, std::equal_to<int>, Alloc> map1(0, Alloc{1});
        map1.Insert({1, 10});
        ASSERT_GT(map1.GetAllocator().allocations_, 0);

        UnorderedMap<int, int, std::hash<int>, std::equal_to<int>, Alloc> map2(0, Alloc{2});
        map2 = map1;

        AssertMapContains(map2, {{1, 10}});
        ASSERT_EQ(map2.GetAllocator().value_, 2);
        ASSERT_GT(map2.GetAllocator().allocations_, 0);
    }
}

TEST(UnorderedMap, MoveAssignmentAlloc) {
    {
        using Alloc =
            PropagatableCountingAllocator<std::pair<const int, int>, false, true, true, false>;

        UnorderedMap<int, int, std::hash<int>, std::equal_to<int>, Alloc> src(0, Alloc{1});
        src.Insert({1, 10});
        src.Insert({2, 20});

        UnorderedMap<int, int, std::hash<int>, std::equal_to<int>, Alloc> dst(0, Alloc{2});
        dst.Insert({100, 1000});

        dst = std::move(src);

        AssertMapContains(dst, {{1, 10}, {2, 20}});
        ASSERT_EQ(dst.GetAllocator().value_, 1);
    }

    {
        using Alloc =
            PropagatableCountingAllocator<std::pair<const int, int>, false, false, true, false>;

        UnorderedMap<int, int, std::hash<int>, std::equal_to<int>, Alloc> src(0, Alloc{1});
        src.Insert({1, 10});
        src.Insert({2, 20});

        {
            UnorderedMap<int, int, std::hash<int>, std::equal_to<int>, Alloc> dst(0, Alloc{2});
            dst.Insert({100, 1000});

            dst = std::move(src);

            AssertMapContains(dst, {{1, 10}, {2, 20}});
            ASSERT_EQ(dst.GetAllocator().value_, 2);
        }
    }

    {
        using Alloc =
            PropagatableCountingAllocator<std::pair<const int, int>, false, false, true, true>;

        UnorderedMap<int, int, std::hash<int>, std::equal_to<int>, Alloc> src(0, Alloc{1});
        src.Insert({1, 10});

        UnorderedMap<int, int, std::hash<int>, std::equal_to<int>, Alloc> dst(0, Alloc{2});
        dst = std::move(src);

        AssertMapContains(dst, {{1, 10}});
        ASSERT_EQ(dst.GetAllocator().value_, 2);
    }
}

TEST(UnorderedMap, SwapAlloc) {
    {
        using Alloc = PropagatableCountingAllocator<std::pair<const int, int>, true, false, true>;

        UnorderedMap<int, int, std::hash<int>, std::equal_to<int>, Alloc> map1(0, Alloc{1});
        map1.Insert({1, 10});

        UnorderedMap<int, int, std::hash<int>, std::equal_to<int>, Alloc> map2(0, Alloc{2});
        map2.Insert({2, 20});

        map1.Swap(map2);

        AssertMapContains(map1, {{2, 20}});
        AssertMapContains(map2, {{1, 10}});

        ASSERT_EQ(map1.GetAllocator().value_, 2);
        ASSERT_EQ(map2.GetAllocator().value_, 1);
    }

    {
        using Alloc = PropagatableCountingAllocator<std::pair<const int, int>, true, false, false>;

        UnorderedMap<int, int, std::hash<int>, std::equal_to<int>, Alloc> map1(0, Alloc{1});
        map1.Insert({1, 10});

        UnorderedMap<int, int, std::hash<int>, std::equal_to<int>, Alloc> map2(0, Alloc{2});
        map2.Insert({2, 20});

        map1.Swap(map2);

        AssertMapContains(map1, {{2, 20}});
        AssertMapContains(map2, {{1, 10}});

        ASSERT_EQ(map1.GetAllocator().value_, 1);
        ASSERT_EQ(map2.GetAllocator().value_, 2);
    }
}

TEST(UnorderedMap, SwapDoesNotInvalidateIterators) {
    using Alloc = PropagatableCountingAllocator<std::pair<const int, int>, true, false, true>;

    UnorderedMap<int, int, std::hash<int>, std::equal_to<int>, Alloc> map1(0, Alloc{1});
    UnorderedMap<int, int, std::hash<int>, std::equal_to<int>, Alloc> map2(0, Alloc{2});

    map1.Insert({1, 10});
    map2.Insert({2, 20});

    auto it1 = map1.Find(1);
    auto it2 = map2.Find(2);

    const auto* ptr1 = &(*it1);
    const auto* ptr2 = &(*it2);

    map1.Swap(map2);

    ASSERT_EQ(ptr1->first, 1);
    ASSERT_EQ(ptr1->second, 10);
    ASSERT_EQ(ptr2->first, 2);
    ASSERT_EQ(ptr2->second, 20);

    ASSERT_NE(map2.Find(1), map2.end());
    ASSERT_NE(map1.Find(2), map1.end());
}

}  // namespace additional

struct HashCounter {
    static size_t calls;

    static void Reset() {
        calls = 0;
    }

    size_t operator()(int x) const {
        ++calls;
        return std::hash<int>{}(x);
    }
};

size_t HashCounter::calls = 0;

struct ConstantHasher {
    size_t operator()(int) const noexcept {
        return 42;
    }
};

TEST(OptimizationTest, NoHashCallsDuringRehash) {
    UnorderedMap<int, int, HashCounter> map;
    HashCounter::Reset();

    const int elements = 100;
    for (int i = 0; i < elements; ++i) {
        map.Insert({i, i});
    }

    size_t calls_after_insert = HashCounter::calls;
    map.Reserve(100000);

    EXPECT_EQ(HashCounter::calls, calls_after_insert)
        << "Hash function must not be called during rehash";

    for (int i = 0; i < elements; ++i) {
        EXPECT_EQ(map.At(i), i);
    }
}

TEST(EfficiencyTest, HeavyCollisionScenario) {
    UnorderedMap<int, int, ConstantHasher> map;
    const int elements = 1000;

    for (int i = 0; i < elements; ++i) {
        map.Insert({i, i * 10});
    }

    EXPECT_EQ(map.Size(), static_cast<size_t>(elements));

    for (int i = 0; i < elements; ++i) {
        auto it = map.Find(i);
        ASSERT_NE(it, map.end());
        EXPECT_EQ(it->second, i * 10);
    }

    for (int i = 0; i < elements; i += 2) {
        auto it = map.Find(i);
        auto cur = it++;
        EXPECT_EQ(map.Erase(cur), it);
    }

    EXPECT_EQ(map.Size(), static_cast<size_t>(elements / 2));

    for (int i = 1; i < elements; i += 2) {
        EXPECT_TRUE(map.Contains(i));
        EXPECT_EQ(map.At(i), i * 10);
    }
}

enum class FuzzingOpType {
    Insert,
    Emplace,
    Subscript,
    Find,
    Contains,
    At,
    Clear,
    Size,
    Iteration,
};

void RunFuzzyTestOperations(size_t seed) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> key_dist(1, 1000);
    std::uniform_int_distribution<int> value_dist(1, 10000);
    std::uniform_int_distribution<int> op_dist(1, 9);

    UnorderedMap<int, int> my_map;
    std::unordered_map<int, int> std_map;

    const int operations_count = 50000;

    for (int i = 0; i < operations_count; ++i) {
        FuzzingOpType op = static_cast<FuzzingOpType>(op_dist(rng));
        int key = key_dist(rng);
        int value = value_dist(rng);

        switch (op) {
            case FuzzingOpType::Insert: {
                auto r1 = my_map.Insert({key, value});
                auto r2 = std_map.insert({key, value});
                ASSERT_EQ(r1.second, r2.second);
                break;
            }
            case FuzzingOpType::Emplace: {
                auto r1 = my_map.Emplace(key, value);
                auto r2 = std_map.emplace(key, value);
                ASSERT_EQ(r1.second, r2.second);
                break;
            }
            case FuzzingOpType::Subscript:
                my_map[key] = value;
                std_map[key] = value;
                break;
            case FuzzingOpType::Find: {
                auto it1 = my_map.Find(key);
                auto it2 = std_map.find(key);
                ASSERT_EQ(it1 != my_map.end(), it2 != std_map.end());
                if (it1 != my_map.end()) {
                    ASSERT_EQ(it1->second, it2->second);
                }
                break;
            }
            case FuzzingOpType::Contains:
                ASSERT_EQ(my_map.Contains(key), std_map.find(key) != std_map.end());
                break;
            case FuzzingOpType::At:
                if (std_map.find(key) != std_map.end()) {
                    ASSERT_EQ(my_map.At(key), std_map.at(key));
                } else {
                    EXPECT_THROW(my_map.At(key), std::out_of_range);
                }
                break;
            case FuzzingOpType::Clear:
                if (i % 100 == 0) {
                    my_map.Clear();
                    std_map.clear();
                }
                break;
            case FuzzingOpType::Size:
                ASSERT_EQ(my_map.Size(), std_map.size());
                ASSERT_EQ(my_map.Empty(), std_map.empty());
                break;
            case FuzzingOpType::Iteration: {
                int sum1 = 0, sum2 = 0;
                for (const auto& kv : my_map) {
                    sum1 += kv.second;
                }
                for (const auto& kv : std_map) {
                    sum2 += kv.second;
                }
                ASSERT_EQ(sum1, sum2);
                break;
            }
        }
    }

    ASSERT_EQ(my_map.Size(), std_map.size());
}

TEST(FuzzyTest, BasicOperations) {
    RunFuzzyTestOperations(537);
}
