
#include <gtest/gtest.h>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include "unq_ptr.h"
#include "shrd_ptr.h"

class ThrowingObject {
private:
    int value;
public:
    static int alive_count;
    explicit ThrowingObject(int value) : value(value) {
        alive_count++;
    }
    ThrowingObject(const ThrowingObject& other) : value(other.value) {
        throw std::runtime_error("Copy failed");
    }
    ~ThrowingObject() {
        alive_count--;
    }
    int get_value() const {
        return value;
    }
};

int ThrowingObject::alive_count = 0;

class DestructionBase {
public:
    static int destroyed_count;
    virtual ~DestructionBase() {
        destroyed_count++;
    }
};

int DestructionBase::destroyed_count = 0;

class DestructionDerived : public DestructionBase {
public:
    static int derived_destroyed_count;
    ~DestructionDerived() override {
        derived_destroyed_count++;
    }
};

int DestructionDerived::derived_destroyed_count = 0;

class LifetimeTracker {
public:
    static int alive_count;
    int value;
    explicit LifetimeTracker(int value = 0) : value(value) {
        alive_count++;
    }
    LifetimeTracker(const LifetimeTracker& other) : value(other.value) {
        alive_count++;
    }
    ~LifetimeTracker() {
        alive_count--;
    }
};

int LifetimeTracker::alive_count = 0;

TEST(SafetyTest, UnqPtrCopyingIsForbidden) {
    EXPECT_FALSE(std::is_copy_constructible_v<UnqPtr<ThrowingObject>>);
    EXPECT_FALSE(std::is_copy_assignable_v<UnqPtr<ThrowingObject>>);
}

TEST(SafetyTest, UnqPtrMoveDoesNotCopyObject) {
    EXPECT_EQ(ThrowingObject::alive_count, 0);
    {
        UnqPtr<ThrowingObject> first(new ThrowingObject(10));
        EXPECT_EQ(ThrowingObject::alive_count, 1);
        UnqPtr<ThrowingObject> second(std::move(first));
        EXPECT_TRUE(first.is_null());
        EXPECT_EQ(second->get_value(), 10);
        EXPECT_EQ(ThrowingObject::alive_count, 1);
    }
    EXPECT_EQ(ThrowingObject::alive_count, 0);
}

TEST(SafetyTest, UnqPtrMoveAssignmentReleasesOldObject) {
    EXPECT_EQ(LifetimeTracker::alive_count, 0);
    {
        UnqPtr<LifetimeTracker> first(new LifetimeTracker(10));
        UnqPtr<LifetimeTracker> second(new LifetimeTracker(20));
        EXPECT_EQ(LifetimeTracker::alive_count, 2);
        second = std::move(first);
        EXPECT_TRUE(first.is_null());
        EXPECT_EQ(second->value, 10);
        EXPECT_EQ(LifetimeTracker::alive_count, 1);
    }
    EXPECT_EQ(LifetimeTracker::alive_count, 0);
}

TEST(SafetyTest, ShrdPtrDeletesDerivedObjectOnce) {
    DestructionBase::destroyed_count = 0;
    DestructionDerived::derived_destroyed_count = 0;
    {
        ShrdPtr<DestructionDerived> derived(new DestructionDerived());
        {
            ShrdPtr<DestructionBase> base(derived);
            EXPECT_EQ(base.use_count(), 2);
        }
        EXPECT_EQ(DestructionDerived::derived_destroyed_count, 0);
    }
    EXPECT_EQ(DestructionDerived::derived_destroyed_count, 1);
    EXPECT_EQ(DestructionBase::destroyed_count, 1);
}

TEST(SafetyTest, ShrdPtrAssignmentReleasesOldObject) {
    DestructionBase::destroyed_count = 0;
    {
        ShrdPtr<DestructionBase> first(new DestructionBase());
        ShrdPtr<DestructionBase> second(new DestructionBase());
        second = first;
        EXPECT_EQ(DestructionBase::destroyed_count, 1);
        EXPECT_EQ(first.use_count(), 2);
    }
    EXPECT_EQ(DestructionBase::destroyed_count, 2);
}

TEST(SafetyTest, ShrdPtrMultipleCopiesDeleteOnce) {
    DestructionBase::destroyed_count = 0;
    {
        ShrdPtr<DestructionBase> first(new DestructionBase());
        ShrdPtr<DestructionBase> second(first);
        ShrdPtr<DestructionBase> third(second);
        EXPECT_EQ(first.use_count(), 3);
    }
    EXPECT_EQ(DestructionBase::destroyed_count, 1);
}

TEST(SafetyTest, SharedPointerDestroysObjectOnce) {
    EXPECT_EQ(LifetimeTracker::alive_count, 0);
    {
        ShrdPtr<LifetimeTracker> first(new LifetimeTracker(10));
        {
            ShrdPtr<LifetimeTracker> second(first);
            ShrdPtr<LifetimeTracker> third(second);
            EXPECT_EQ(first.use_count(), 3);
            EXPECT_EQ(LifetimeTracker::alive_count, 1);
        }
        EXPECT_EQ(first.use_count(), 1);
        EXPECT_EQ(LifetimeTracker::alive_count, 1);
    }
    EXPECT_EQ(LifetimeTracker::alive_count, 0);
}

TEST(SafetyTest, UniquePointerMoveDestroysObjectOnce) {
    EXPECT_EQ(LifetimeTracker::alive_count, 0);
    {
        UnqPtr<LifetimeTracker> first(new LifetimeTracker(10));
        UnqPtr<LifetimeTracker> second(std::move(first));
        UnqPtr<LifetimeTracker> third(std::move(second));
        EXPECT_TRUE(first.is_null());
        EXPECT_TRUE(second.is_null());
        EXPECT_EQ(third->value, 10);
        EXPECT_EQ(LifetimeTracker::alive_count, 1);
    }
    EXPECT_EQ(LifetimeTracker::alive_count, 0);
}

TEST(SafetyTest, SharedPointerMoveAssignmentReleasesOldObject) {
    EXPECT_EQ(LifetimeTracker::alive_count, 0);
    {
        ShrdPtr<LifetimeTracker> first(new LifetimeTracker(10));
        ShrdPtr<LifetimeTracker> second(new LifetimeTracker(20));
        EXPECT_EQ(LifetimeTracker::alive_count, 2);
        second = std::move(first);
        EXPECT_TRUE(first.is_null());
        EXPECT_EQ(second.use_count(), 1);
        EXPECT_EQ(second->value, 10);
        EXPECT_EQ(LifetimeTracker::alive_count, 1);
    }
    EXPECT_EQ(LifetimeTracker::alive_count, 0);
}

TEST(SafetyTest, SharedPointerCopyAssignmentReleasesOldObject) {
    EXPECT_EQ(LifetimeTracker::alive_count, 0);
    {
        ShrdPtr<LifetimeTracker> first(new LifetimeTracker(10));
        ShrdPtr<LifetimeTracker> second(new LifetimeTracker(20));
        second = first;
        EXPECT_EQ(first.use_count(), 2);
        EXPECT_EQ(second.use_count(), 2);
        EXPECT_EQ(LifetimeTracker::alive_count, 1);
    }
    EXPECT_EQ(LifetimeTracker::alive_count, 0);
}

TEST(SafetyTest, SharedPointerSelfAssignmentKeepsObject) {
    EXPECT_EQ(LifetimeTracker::alive_count, 0);
    {
        ShrdPtr<LifetimeTracker> pointer(new LifetimeTracker(10));
        ShrdPtr<LifetimeTracker>& reference = pointer;
        reference = pointer;
        EXPECT_EQ(pointer.use_count(), 1);
        EXPECT_EQ(pointer->value, 10);
        EXPECT_EQ(LifetimeTracker::alive_count, 1);
    }
    EXPECT_EQ(LifetimeTracker::alive_count, 0);
}
