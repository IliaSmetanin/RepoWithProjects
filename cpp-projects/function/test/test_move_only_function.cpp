#include "function.hpp"

#include "test/common.hpp"

TEST(MoveOnlyFunctionTest, StaticAssertions) {
    TestStaticAssertions<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, Works) {
    TestWorks<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, Template) {
    TestTemplate<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, Lambda) {
    TestLambda<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, Pointer) {
    TestPointer<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, FieldsMethods) {
    TestFieldsAndMethods<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, ChangeSimpleObject) {
    TestChangeSimpleObject<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, Assignment) {
    TestAssignmentOperator<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, ChangeObject) {
    TestChangeObject<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, Empty) {
    TestEmpty<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, Invoke) {
    TestInvoke<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, SOO) {
    TestSmallObjectOptimization<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, StateChange) {
    TestCallChangesState<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, Bind) {
    TestBind<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, NotCopyable) {
    struct NotCopyable {
        std::unique_ptr<int> p{new int(5)};

        int operator()(int x) {
            return x + *p;
        }
    };

    NotCopyable nc;
    MoveOnlyFunction<int(int)> f = std::move(nc);
    ASSERT_EQ(f(5), 10);

    MoveOnlyFunction<int(int)> f2 = std::move(f);
    f = NotCopyable{std::make_unique<int>(7)};

    ASSERT_EQ(f(5), 12);
    ASSERT_EQ(f2(5), 10);
}

TEST(MoveOnlyFunctionTest, AssignEmpty) {
    TestAssignEmpty<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, MoveAssignmentWithState) {
    TestMoveAssignmentWithState<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, ReferenceWrapperAssignment) {
    TestReferenceWrapperAssignment<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, SmallObjectBasic) {
    TestSmallObjectBasic<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, AssignSmallObjectNoAllocation) {
    TestAssignSmallObjectNoAllocation<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, TargetTypeEmpty) {
    TestTargetTypeEmpty<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, TargetTypeFunctionPointer) {
    TestTargetTypeFunctionPointer<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, TargetTypeLambda) {
    TestTargetTypeLambda<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, TargetTypeFunctor) {
    TestTargetTypeFunctor<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, TargetTypeChangesOnAssign) {
    TestTargetTypeChangesOnAssign<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, TargetReturnsStoredFunctionPointer) {
    TestTargetReturnsStoredFunctionPointer<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, TargetReturnsStoredFunctor) {
    TestTargetReturnsStoredFunctor<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, TargetReturnsStoredLambda) {
    TestTargetReturnsStoredLambda<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, TargetForMemberPointer) {
    TestTargetForMemberPointer<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, TargetForReferenceWrapper) {
    TestTargetForReferenceWrapper<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, TargetWrongTypeReturnsNull) {
    TestTargetWrongTypeReturnsNull<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, TargetEmptyReturnsNull) {
    TestTargetEmptyReturnsNull<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, TargetAllowsMutation) {
    TestTargetAllowsMutation<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, TargetConstReturnsConstPointer) {
    TestTargetConstReturnsConstPointer<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, TargetCopyIsIndependent) {
    TestTargetCopyIsIndependent<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, TargetAfterReassignment) {
    TestTargetAfterReassignment<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, TargetAfterMove) {
    TestTargetAfterMove<MoveOnlyFunction, true>();
}

TEST(MoveOnlyFunctionTest, TargetForLargeFunctor) {
    TestTargetForLargeFunctor<MoveOnlyFunction, true>();
}