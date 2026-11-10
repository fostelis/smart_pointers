
#include <gtest/gtest.h>
#include <stdexcept>
#include <utility>
#include "shrd_ptr.h"

class SharedTestObject {
private:
    int value;
public:
    static int alive_count;
    explicit SharedTestObject(int value) : value(value) {
        alive_count++;
    }
    ~SharedTestObject() {
        alive_count--;
    }
    int get_value() const {
        return value;
    }
    void set_value(int new_value) {
        value = new_value;
    }
};

int SharedTestObject::alive_count = 0;

TEST(ShrdPtrTest, DefaultConstructor) {
    ShrdPtr<int> ptr;
    EXPECT_TRUE(ptr.is_null());
    EXPECT_EQ(ptr.use_count(), 0);
}

TEST(ShrdPtrTest, ConstructorStoresObject) {
    ShrdPtr<int> ptr(new int(10));
    EXPECT_FALSE(ptr.is_null());
    EXPECT_EQ(*ptr, 10);
    EXPECT_EQ(ptr.use_count(), 1);
}

TEST(ShrdPtrTest, CopyConstructorIncreasesCount) {
    ShrdPtr<int> first(new int(10));
    ShrdPtr<int> second(first);
    EXPECT_EQ(first.use_count(), 2);
    EXPECT_EQ(second.use_count(), 2);
}

TEST(ShrdPtrTest, CopySharesObject) {
    ShrdPtr<int> first(new int(10));
    ShrdPtr<int> second(first);
    *second = 20;
    EXPECT_EQ(*first, 20);
    EXPECT_EQ(*second, 20);
}

TEST(ShrdPtrTest, CopyAssignment) {
    ShrdPtr<int> first(new int(10));
    ShrdPtr<int> second(new int(20));
    second = first;
    EXPECT_EQ(*second, 10);
    EXPECT_EQ(first.use_count(), 2);
    EXPECT_EQ(second.use_count(), 2);
}

TEST(ShrdPtrTest, SelfAssignment) {
    ShrdPtr<int> ptr(new int(10));
    ptr = ptr;
    EXPECT_EQ(*ptr, 10);
    EXPECT_EQ(ptr.use_count(), 1);
}

TEST(ShrdPtrTest, MoveConstructor) {
    ShrdPtr<int> first(new int(10));
    ShrdPtr<int> second(std::move(first));
    EXPECT_TRUE(first.is_null());
    EXPECT_EQ(first.use_count(), 0);
    EXPECT_EQ(*second, 10);
    EXPECT_EQ(second.use_count(), 1);
}

TEST(ShrdPtrTest, MoveAssignment) {
    ShrdPtr<int> first(new int(10));
    ShrdPtr<int> second(new int(20));
    second = std::move(first);
    EXPECT_TRUE(first.is_null());
    EXPECT_EQ(*second, 10);
    EXPECT_EQ(second.use_count(), 1);
}

TEST(ShrdPtrTest, NullPointerThrows) {
    ShrdPtr<int> ptr;
    EXPECT_THROW(*ptr, std::runtime_error);
}

TEST(ShrdPtrTest, NullArrowThrows) {
    ShrdPtr<SharedTestObject> ptr;
    EXPECT_THROW(ptr.operator->(), std::runtime_error);
}

TEST(ShrdPtrTest, ArrowOperator) {
    ShrdPtr<SharedTestObject> ptr(new SharedTestObject(10));
    EXPECT_EQ(ptr->get_value(), 10);
    ptr->set_value(20);
    EXPECT_EQ(ptr->get_value(), 20);
}

TEST(ShrdPtrTest, DestructorDecreasesCount) {
    ShrdPtr<int> first(new int(10));
    {
        ShrdPtr<int> second(first);
        EXPECT_EQ(first.use_count(), 2);
    }
    EXPECT_EQ(first.use_count(), 1);
}

TEST(ShrdPtrTest, LastPointerDeletesObject) {
    EXPECT_EQ(SharedTestObject::alive_count, 0);
    {
        ShrdPtr<SharedTestObject> first(new SharedTestObject(10));
        {
            ShrdPtr<SharedTestObject> second(first);
            EXPECT_EQ(SharedTestObject::alive_count, 1);
        }
        EXPECT_EQ(SharedTestObject::alive_count, 1);
    }
    EXPECT_EQ(SharedTestObject::alive_count, 0);
}

TEST(ShrdPtrTest, CopyNullPointer) {
    ShrdPtr<int> first;
    ShrdPtr<int> second(first);
    EXPECT_TRUE(second.is_null());
    EXPECT_EQ(second.use_count(), 0);
}

TEST(ShrdPtrTest, AssignNullPointer) {
    ShrdPtr<int> first(new int(10));
    ShrdPtr<int> second;
    first = second;
    EXPECT_TRUE(first.is_null());
    EXPECT_EQ(first.use_count(), 0);
}

TEST(ShrdPtrTest, MultipleReferences) {
    ShrdPtr<int> first(new int(10));
    ShrdPtr<int> second(first);
    ShrdPtr<int> third(second);
    EXPECT_EQ(first.use_count(), 3);
    EXPECT_EQ(second.use_count(), 3);
    EXPECT_EQ(third.use_count(), 3);
}
