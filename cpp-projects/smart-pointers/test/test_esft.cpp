#include "smart_pointers.hpp"

#include <memory>

#include <gtest/gtest.h>

struct Enabled : public EnableSharedFromThis<Enabled> {
    SharedPtr<Enabled> GetShared() {
        return SharedFromThis();
    }

    SharedPtr<const Enabled> GetSharedConst() const {
        return SharedFromThis();
    }
};

TEST(EnableSharedFromThis, ManualManagement) {
    Enabled e;
    ASSERT_THROW(e.GetShared(), std::bad_weak_ptr);
}

TEST(EnableSharedFromThis, Works) {
    auto esp = MakeShared<Enabled>();

    auto& e = *esp;
    auto sp = e.GetShared();

    ASSERT_EQ(sp.UseCount(), 2);

    esp.Reset();
    ASSERT_EQ(sp.UseCount(), 1);

    sp.Reset();
}

TEST(EnableSharedFromThis, ManualAllocation) {
    SharedPtr<Enabled> esp(new Enabled());

    auto& e = *esp;
    auto sp = e.GetShared();

    ASSERT_EQ(sp.UseCount(), 2);

    esp.Reset();
    ASSERT_EQ(sp.UseCount(), 1);

    sp.Reset();
}

TEST(EnableSharedFromThis, Const) {
    auto esp = MakeShared<Enabled>();

    auto& e = *esp;
    auto sp = e.GetShared();
    ASSERT_EQ(sp.UseCount(), 2);

    const auto& ce = e;
    auto sp_const = ce.GetSharedConst();
    ASSERT_EQ(sp_const.UseCount(), 3);

    esp.Reset();
    ASSERT_EQ(sp.UseCount(), 2);

    sp.Reset();
    sp_const.Reset();
}