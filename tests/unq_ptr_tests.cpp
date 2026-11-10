
#include <gtest/gtest.h>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include "unq_ptr.h"

class TestObject {
private:
    int value;
public:
    static int alive_count;
    explicit TestObject(int value) : value(value) {
        alive_count++;
    }
    TestObject(const TestObject& other) : value(other.value) {
        alive_count++;
    }
    ~TestObject() {
        alive_count--;
    }
    int get_value() const {
        return value;
    }
    void set_value(int new_value) {
        value = new_value;
    }
};

int TestObject::alive_count = 0;

TEST(UnqPtrTest, DefaultConstructorCreatesNullPointer) {
    UnqPtr<int> ptr;
    EXPECT_TRUE(ptr.is_null());
    EXPECT_EQ(ptr.get(), nullptr);
}

TEST(UnqPtrTest, ConstructorStoresObject) {
    UnqPtr<int> ptr(new int(10));
    EXPECT_FALSE(ptr.is_null());
    EXPECT_EQ(*ptr, 10);
}

TEST(UnqPtrTest, CopyConstructorIsForbidden) {
    EXPECT_FALSE(std::is_copy_constructible_v<UnqPtr<int>>);
    EXPECT_FALSE(std::is_copy_constructible_v<UnqPtr<TestObject>>);
}

TEST(UnqPtrTest, CopyAssignmentIsForbidden) {
    EXPECT_FALSE(std::is_copy_assignable_v<UnqPtr<int>>);
    EXPECT_FALSE(std::is_copy_assignable_v<UnqPtr<TestObject>>);
}

TEST(UnqPtrTest, MoveConstructorTransfersOwnership) {
    UnqPtr<int> first(new int(10));
    int* original = first.get();
    UnqPtr<int> second(std::move(first));
    EXPECT_TRUE(first.is_null());
    EXPECT_EQ(second.get(), original);
    EXPECT_EQ(*second, 10);
}

TEST(UnqPtrTest, MoveAssignmentTransfersOwnership) {
    UnqPtr<int> first(new int(10));
    UnqPtr<int> second(new int(20));
    int* original = first.get();
    second = std::move(first);
    EXPECT_TRUE(first.is_null());
    EXPECT_EQ(second.get(), original);
    EXPECT_EQ(*second, 10);
}

TEST(UnqPtrTest, SelfMoveAssignmentKeepsObject) {
    UnqPtr<int> ptr(new int(10));
    auto& same = ptr;
    ptr = std::move(same);
    EXPECT_FALSE(ptr.is_null());
    EXPECT_EQ(*ptr, 10);
}

TEST(UnqPtrTest, DereferenceOperatorReturnsObject) {
    UnqPtr<int> ptr(new int(10));
    EXPECT_EQ(*ptr, 10);
    *ptr = 25;
    EXPECT_EQ(*ptr, 25);
}

TEST(UnqPtrTest, ArrowOperatorProvidesAccessToObject) {
    UnqPtr<TestObject> ptr(new TestObject(10));
    EXPECT_EQ(ptr->get_value(), 10);
    ptr->set_value(20);
    EXPECT_EQ(ptr->get_value(), 20);
}

TEST(UnqPtrTest, GetReturnsStoredPointer) {
    UnqPtr<int> ptr(new int(10));
    int* raw_ptr = ptr.get();
    EXPECT_NE(raw_ptr, nullptr);
    EXPECT_EQ(*raw_ptr, 10);
}

TEST(UnqPtrTest, DereferenceNullPointerThrowsException) {
    UnqPtr<int> ptr;
    EXPECT_THROW(*ptr, std::runtime_error);
}

TEST(UnqPtrTest, ArrowOperatorOnNullPointerThrowsException) {
    UnqPtr<TestObject> ptr;
    EXPECT_THROW(ptr.operator->(), std::runtime_error);
}

TEST(UnqPtrTest, DestructorDeletesObject) {
    EXPECT_EQ(TestObject::alive_count, 0);
    {
        UnqPtr<TestObject> ptr(new TestObject(10));
        EXPECT_EQ(TestObject::alive_count, 1);
    }
    EXPECT_EQ(TestObject::alive_count, 0);
}

TEST(UnqPtrTest, MoveDoesNotCreateAnotherObject) {
    EXPECT_EQ(TestObject::alive_count, 0);
    {
        UnqPtr<TestObject> first(new TestObject(10));
        EXPECT_EQ(TestObject::alive_count, 1);
        UnqPtr<TestObject> second(std::move(first));
        EXPECT_TRUE(first.is_null());
        EXPECT_EQ(TestObject::alive_count, 1);
        EXPECT_EQ(second->get_value(), 10);
    }
    EXPECT_EQ(TestObject::alive_count, 0);
}

TEST(UnqPtrTest, ReleaseTransfersResponsibilityToCaller) {
    UnqPtr<int> ptr(new int(42));
    int* raw_ptr = ptr.release();
    EXPECT_TRUE(ptr.is_null());
    ASSERT_NE(raw_ptr, nullptr);
    EXPECT_EQ(*raw_ptr, 42);
    delete raw_ptr;
}

TEST(UnqPtrTest, ResetReplacesObject) {
    EXPECT_EQ(TestObject::alive_count, 0);
    {
        UnqPtr<TestObject> ptr(new TestObject(10));
        EXPECT_EQ(TestObject::alive_count, 1);
        ptr.reset(new TestObject(20));
        EXPECT_EQ(TestObject::alive_count, 1);
        EXPECT_EQ(ptr->get_value(), 20);
        ptr.reset();
        EXPECT_EQ(TestObject::alive_count, 0);
        EXPECT_TRUE(ptr.is_null());
    }
    EXPECT_EQ(TestObject::alive_count, 0);
}

TEST(UnqPtrTest, BooleanConversionWorks) {
    UnqPtr<int> empty;
    UnqPtr<int> filled(new int(5));
    EXPECT_FALSE(static_cast<bool>(empty));
    EXPECT_TRUE(static_cast<bool>(filled));
}

TEST(UnqPtrArrayTest, ConstructorStoresArray) {
    UnqPtr<int[]> ptr(new int[3]{10, 20, 30});
    EXPECT_FALSE(ptr.is_null());
    EXPECT_EQ(ptr[0], 10);
    EXPECT_EQ(ptr[1], 20);
    EXPECT_EQ(ptr[2], 30);
}

TEST(UnqPtrArrayTest, IndexOperatorAllowsModification) {
    UnqPtr<int[]> ptr(new int[3]{1, 2, 3});
    ptr[1] = 99;
    EXPECT_EQ(ptr[1], 99);
}

TEST(UnqPtrArrayTest, CopyingIsForbidden) {
    EXPECT_FALSE(std::is_copy_constructible_v<UnqPtr<int[]>>);
    EXPECT_FALSE(std::is_copy_assignable_v<UnqPtr<int[]>>);
}

TEST(UnqPtrArrayTest, MoveConstructorTransfersOwnership) {
    UnqPtr<int[]> first(new int[3]{1, 2, 3});
    int* original = first.get();
    UnqPtr<int[]> second(std::move(first));
    EXPECT_TRUE(first.is_null());
    EXPECT_EQ(second.get(), original);
    EXPECT_EQ(second[2], 3);
}

TEST(UnqPtrArrayTest, MoveAssignmentTransfersOwnership) {
    UnqPtr<int[]> first(new int[2]{10, 20});
    UnqPtr<int[]> second(new int[2]{30, 40});
    second = std::move(first);
    EXPECT_TRUE(first.is_null());
    EXPECT_EQ(second[0], 10);
    EXPECT_EQ(second[1], 20);
}

TEST(UnqPtrArrayTest, DestructorDeletesEveryElement) {
    EXPECT_EQ(TestObject::alive_count, 0);
    {
        UnqPtr<TestObject[]> ptr(new TestObject[3]{
            TestObject(1), TestObject(2), TestObject(3)
        });
        EXPECT_EQ(TestObject::alive_count, 3);
        EXPECT_EQ(ptr[1].get_value(), 2);
    }
    EXPECT_EQ(TestObject::alive_count, 0);
}

TEST(UnqPtrArrayTest, ResetDeletesPreviousArray) {
    EXPECT_EQ(TestObject::alive_count, 0);
    {
        UnqPtr<TestObject[]> ptr(new TestObject[2]{
            TestObject(1), TestObject(2)
        });
        EXPECT_EQ(TestObject::alive_count, 2);
        ptr.reset(new TestObject[3]{
            TestObject(3), TestObject(4), TestObject(5)
        });
        EXPECT_EQ(TestObject::alive_count, 3);
        ptr.reset();
        EXPECT_EQ(TestObject::alive_count, 0);
    }
    EXPECT_EQ(TestObject::alive_count, 0);
}

TEST(UnqPtrArrayTest, ReleaseReturnsArrayWithoutDeletingIt) {
    UnqPtr<int[]> ptr(new int[2]{10, 20});
    int* raw_ptr = ptr.release();
    EXPECT_TRUE(ptr.is_null());
    ASSERT_NE(raw_ptr, nullptr);
    EXPECT_EQ(raw_ptr[1], 20);
    delete[] raw_ptr;
}

TEST(UnqPtrArrayTest, AccessingNullArrayThrowsException) {
    UnqPtr<int[]> ptr;
    EXPECT_THROW(ptr[0], std::runtime_error);
}
