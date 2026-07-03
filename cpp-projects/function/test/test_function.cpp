#include "function.hpp"

#include "test/common.hpp"

TEST(FunctionTest, StaticAssertions) {
    TestStaticAssertions<Function, false>();
}

TEST(FunctionTest, Works) {
    TestWorks<Function, false>();
}

TEST(FunctionTest, Template) {
    TestTemplate<Function, false>();
}

TEST(FunctionTest, Lambda) {
    TestLambda<Function, false>();
}

TEST(FunctionTest, Pointer) {
    TestPointer<Function, false>();
}

TEST(FunctionTest, FieldsMethods) {
    TestFieldsAndMethods<Function, false>();
}

TEST(FunctionTest, ChangeSimpleObject) {
    TestChangeSimpleObject<Function, false>();
}

TEST(FunctionTest, Assignment) {
    TestAssignmentOperator<Function, false>();
}

TEST(FunctionTest, ChangeObject) {
    TestChangeObject<Function, false>();
}

TEST(FunctionTest, Empty) {
    TestEmpty<Function, false>();
}

TEST(FunctionTest, Invoke) {
    TestInvoke<Function, false>();
}

TEST(FunctionTest, SOO) {
    TestSmallObjectOptimization<Function, false>();
}

TEST(FunctionTest, StateChange) {
    TestCallChangesState<Function, false>();
}

TEST(FunctionTest, Bind) {
    TestBind<Function, false>();
}

TEST(FunctionTest, AssignEmpty) {
    TestAssignEmpty<Function, false>();
}

TEST(FunctionTest, MoveAssignmentWithState) {
    TestMoveAssignmentWithState<Function, false>();
}

TEST(FunctionTest, ReferenceWrapperAssignment) {
    TestReferenceWrapperAssignment<Function, false>();
}

TEST(FunctionTest, SmallObjectBasic) {
    TestSmallObjectBasic<Function, false>();
}

TEST(FunctionTest, AssignSmallObjectNoAllocation) {
    TestAssignSmallObjectNoAllocation<Function, false>();
}

TEST(FunctionTest, TargetTypeEmpty) {
    TestTargetTypeEmpty<Function, false>();
}

TEST(FunctionTest, TargetTypeFunctionPointer) {
    TestTargetTypeFunctionPointer<Function, false>();
}

TEST(FunctionTest, TargetTypeLambda) {
    TestTargetTypeLambda<Function, false>();
}

TEST(FunctionTest, TargetTypeFunctor) {
    TestTargetTypeFunctor<Function, false>();
}

TEST(FunctionTest, TargetTypeChangesOnAssign) {
    TestTargetTypeChangesOnAssign<Function, false>();
}

TEST(FunctionTest, TargetReturnsStoredFunctionPointer) {
    TestTargetReturnsStoredFunctionPointer<Function, false>();
}

TEST(FunctionTest, TargetReturnsStoredFunctor) {
    TestTargetReturnsStoredFunctor<Function, false>();
}

TEST(FunctionTest, TargetReturnsStoredLambda) {
    TestTargetReturnsStoredLambda<Function, false>();
}

TEST(FunctionTest, TargetForMemberPointer) {
    TestTargetForMemberPointer<Function, false>();
}

TEST(FunctionTest, TargetForReferenceWrapper) {
    TestTargetForReferenceWrapper<Function, false>();
}

TEST(FunctionTest, TargetWrongTypeReturnsNull) {
    TestTargetWrongTypeReturnsNull<Function, false>();
}

TEST(FunctionTest, TargetEmptyReturnsNull) {
    TestTargetEmptyReturnsNull<Function, false>();
}

TEST(FunctionTest, TargetAllowsMutation) {
    TestTargetAllowsMutation<Function, false>();
}

TEST(FunctionTest, TargetConstReturnsConstPointer) {
    TestTargetConstReturnsConstPointer<Function, false>();
}

TEST(FunctionTest, TargetCopyIsIndependent) {
    TestTargetCopyIsIndependent<Function, false>();
}

TEST(FunctionTest, TargetAfterReassignment) {
    TestTargetAfterReassignment<Function, false>();
}

TEST(FunctionTest, TargetAfterMove) {
    TestTargetAfterMove<Function, false>();
}

TEST(FunctionTest, TargetForLargeFunctor) {
    TestTargetForLargeFunctor<Function, false>();
}

TEST(FunctionTest, CopyAssignmentDeepCopy) {
    AllocatorGuard guard;

    struct Counter {
        int operator()() {
            return ++value;
        }

        int value;
    };

    Function<int()> src = Counter{0};
    Function<int()> dst = Counter{100};

    dst = src;

    ASSERT_EQ(src(), 1);
    ASSERT_EQ(src(), 2);

    ASSERT_EQ(dst(), 1);
    ASSERT_EQ(dst(), 2);
}