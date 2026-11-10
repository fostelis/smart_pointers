
#include <gtest/gtest.h>
#include <vector>
#include <utility>
#include "unq_ptr.h"
#include "shrd_ptr.h"

TEST(ContainerTest, VectorOfUnqPtr) {
    std::vector<UnqPtr<int>> values;
    for (int i = 0; i < 100; i++) {
        values.emplace_back(new int(i));
    }
    EXPECT_EQ(values.size(), 100);
    for (int i = 0; i < 100; i++) {
        EXPECT_EQ(*values[i], i);
    }
}

TEST(ContainerTest, VectorOfShrdPtr) {
    std::vector<ShrdPtr<int>> values;
    for (int i = 0; i < 100; i++) {
        values.emplace_back(new int(i));
    }
    EXPECT_EQ(values.size(), 100);
    for (int i = 0; i < 100; i++) {
        EXPECT_EQ(*values[i], i);
        EXPECT_EQ(values[i].use_count(), 1);
    }
}

TEST(ContainerTest, VectorMovesUnqPtrOwnership) {
    UnqPtr<int> original(new int(10));
    int* original_address = original.get();
    std::vector<UnqPtr<int>> values;
    values.push_back(std::move(original));
    EXPECT_TRUE(original.is_null());
    ASSERT_EQ(values.size(), 1);
    EXPECT_EQ(values[0].get(), original_address);
    EXPECT_EQ(*values[0], 10);
    *values[0] = 20;
    EXPECT_EQ(*values[0], 20);
}

TEST(ContainerTest, VectorSharesShrdPtrOwnership) {
    ShrdPtr<int> original(new int(10));
    std::vector<ShrdPtr<int>> values;
    values.push_back(original);
    values.push_back(original);
    EXPECT_EQ(original.use_count(), 3);
    EXPECT_EQ(values[0].use_count(), 3);
    EXPECT_EQ(values[1].use_count(), 3);
    *values[0] = 20;
    EXPECT_EQ(*original, 20);
    EXPECT_EQ(*values[1], 20);
}

TEST(ContainerTest, VectorReallocationPreservesUnqPtr) {
    std::vector<UnqPtr<int>> values;
    values.emplace_back(new int(42));
    for (int i = 0; i < 1000; i++) {
        values.emplace_back(new int(i));
    }
    EXPECT_EQ(*values[0], 42);
    EXPECT_EQ(values.size(), 1001);
}

TEST(ContainerTest, VectorReallocationPreservesShrdPtr) {
    std::vector<ShrdPtr<int>> values;
    ShrdPtr<int> original(new int(42));
    values.push_back(original);
    for (int i = 0; i < 1000; i++) {
        values.emplace_back(new int(i));
    }
    EXPECT_EQ(*values[0], 42);
    EXPECT_EQ(original.use_count(), 2);
    EXPECT_EQ(values.size(), 1001);
}
