
#include <gtest/gtest.h>
#include <type_traits>
#include <utility>
#include "unq_ptr.h"
#include "shrd_ptr.h"

class Animal {
public:
    virtual ~Animal() = default;
    virtual int get_sound() const {
        return 0;
    }
};

class Dog : public Animal {
public:
    int get_sound() const override {
        return 1;
    }
};

class FirstBase {
public:
    virtual ~FirstBase() = default;
    virtual int first_value() const {
        return 10;
    }
};

class SecondBase {
public:
    virtual ~SecondBase() = default;
    virtual int second_value() const {
        return 20;
    }
};

class MultipleDerived : public FirstBase, public SecondBase {
public:
    int first_value() const override {
        return 100;
    }
    int second_value() const override {
        return 200;
    }
};

TEST(InheritanceTest, UnqPtrSupportsPolymorphism) {
    UnqPtr<Animal> animal(new Dog());
    EXPECT_EQ(animal->get_sound(), 1);
}

TEST(InheritanceTest, ShrdPtrSupportsPolymorphism) {
    ShrdPtr<Animal> animal(new Dog());
    EXPECT_EQ(animal->get_sound(), 1);
}

TEST(InheritanceTest, ShrdPtrCopyPreservesPolymorphism) {
    ShrdPtr<Animal> first(new Dog());
    ShrdPtr<Animal> second(first);
    EXPECT_EQ(first->get_sound(), 1);
    EXPECT_EQ(second->get_sound(), 1);
    EXPECT_EQ(first.use_count(), 2);
}

TEST(InheritanceTest, UnqPtrConvertsDerivedToBaseByMove) {
    UnqPtr<Dog> dog(new Dog());
    Dog* original_address = dog.get();
    UnqPtr<Animal> animal(std::move(dog));
    EXPECT_TRUE(dog.is_null());
    EXPECT_EQ(animal.get(), static_cast<Animal*>(original_address));
    EXPECT_EQ(animal->get_sound(), 1);
}

TEST(InheritanceTest, UnqPtrConversionTransfersOwnership) {
    UnqPtr<Dog> dog(new Dog());
    UnqPtr<Animal> animal(std::move(dog));
    EXPECT_TRUE(dog.is_null());
    EXPECT_FALSE(animal.is_null());
    EXPECT_EQ(animal->get_sound(), 1);
}

TEST(InheritanceTest, UnqPtrConversionAssignment) {
    UnqPtr<Dog> dog(new Dog());
    UnqPtr<Animal> animal;
    animal = std::move(dog);
    EXPECT_TRUE(dog.is_null());
    EXPECT_EQ(animal->get_sound(), 1);
}

TEST(InheritanceTest, UnqPtrConvertsEmptyPointer) {
    UnqPtr<Dog> dog;
    UnqPtr<Animal> animal(std::move(dog));
    EXPECT_TRUE(dog.is_null());
    EXPECT_TRUE(animal.is_null());
}

TEST(InheritanceTest, UnrelatedTypesCannotConvert) {
    EXPECT_FALSE((std::is_constructible_v<UnqPtr<Dog>, UnqPtr<Animal>&&>));
    EXPECT_TRUE((std::is_constructible_v<UnqPtr<Animal>, UnqPtr<Dog>&&>));
    EXPECT_FALSE((std::is_constructible_v<UnqPtr<Animal>, const UnqPtr<Dog>&>));
}

TEST(InheritanceTest, UnqPtrRepeatedMovePreservesDerivedType) {
    UnqPtr<Dog> dog(new Dog());
    UnqPtr<Animal> animal(std::move(dog));
    UnqPtr<Animal> another(std::move(animal));
    EXPECT_TRUE(dog.is_null());
    EXPECT_TRUE(animal.is_null());
    EXPECT_EQ(another->get_sound(), 1);
}

TEST(InheritanceTest, UnqPtrMoveAssignmentPreservesDerivedType) {
    UnqPtr<Dog> dog(new Dog());
    UnqPtr<Animal> animal(std::move(dog));
    UnqPtr<Animal> another;
    another = std::move(animal);
    EXPECT_TRUE(animal.is_null());
    EXPECT_EQ(another->get_sound(), 1);
}

TEST(InheritanceTest, UnqPtrRepeatedMoveKeepsSameObject) {
    UnqPtr<Dog> dog(new Dog());
    Dog* original_address = dog.get();
    UnqPtr<Animal> first(std::move(dog));
    UnqPtr<Animal> second(std::move(first));
    EXPECT_TRUE(dog.is_null());
    EXPECT_TRUE(first.is_null());
    EXPECT_EQ(second.get(), static_cast<Animal*>(original_address));
}

TEST(InheritanceTest, UnqPtrSupportsMultipleInheritance) {
    UnqPtr<MultipleDerived> derived(new MultipleDerived());
    MultipleDerived* original_address = derived.get();
    UnqPtr<SecondBase> base(std::move(derived));
    EXPECT_TRUE(derived.is_null());
    EXPECT_EQ(base->second_value(), 200);
    EXPECT_EQ(base.get(), static_cast<SecondBase*>(original_address));
}

TEST(InheritanceTest, UnqPtrRepeatedMoveWithMultipleInheritance) {
    UnqPtr<MultipleDerived> derived(new MultipleDerived());
    UnqPtr<SecondBase> first(std::move(derived));
    UnqPtr<SecondBase> second(std::move(first));
    EXPECT_TRUE(derived.is_null());
    EXPECT_TRUE(first.is_null());
    EXPECT_EQ(second->second_value(), 200);
}

TEST(InheritanceTest, UnqPtrAssignmentWithMultipleInheritance) {
    UnqPtr<MultipleDerived> derived(new MultipleDerived());
    UnqPtr<SecondBase> base;
    base = std::move(derived);
    EXPECT_TRUE(derived.is_null());
    EXPECT_EQ(base->second_value(), 200);
}

TEST(InheritanceTest, UnqPtrMoveKeepsOriginalType) {
    UnqPtr<MultipleDerived> derived(new MultipleDerived());
    UnqPtr<FirstBase> first(std::move(derived));
    UnqPtr<FirstBase> second(std::move(first));
    EXPECT_TRUE(first.is_null());
    EXPECT_EQ(second->first_value(), 100);
}

TEST(InheritanceTest, ShrdPtrConvertsDerivedToBase) {
    ShrdPtr<Dog> dog(new Dog());
    ShrdPtr<Animal> animal(dog);
    EXPECT_EQ(animal->get_sound(), 1);
    EXPECT_EQ(dog.use_count(), 2);
    EXPECT_EQ(animal.use_count(), 2);
}

TEST(InheritanceTest, ShrdPtrConversionSharesObject) {
    ShrdPtr<Dog> dog(new Dog());
    ShrdPtr<Animal> animal(dog);
    EXPECT_EQ(static_cast<Animal*>(dog.get()), animal.get());
}

TEST(InheritanceTest, ShrdPtrConversionAssignment) {
    ShrdPtr<Dog> dog(new Dog());
    ShrdPtr<Animal> animal;
    animal = dog;
    EXPECT_EQ(animal->get_sound(), 1);
    EXPECT_EQ(animal.use_count(), 2);
}

TEST(InheritanceTest, ShrdPtrConversionMove) {
    ShrdPtr<Dog> dog(new Dog());
    ShrdPtr<Animal> animal(std::move(dog));
    EXPECT_TRUE(dog.is_null());
    EXPECT_EQ(animal->get_sound(), 1);
    EXPECT_EQ(animal.use_count(), 1);
}

TEST(InheritanceTest, ShrdPtrMultipleInheritance) {
    ShrdPtr<MultipleDerived> derived(new MultipleDerived());
    ShrdPtr<SecondBase> base(derived);
    EXPECT_EQ(base->second_value(), 200);
    EXPECT_EQ(base.use_count(), 2);
    EXPECT_EQ(static_cast<SecondBase*>(derived.get()), base.get());
}

TEST(InheritanceTest, ShrdPtrMultipleTypesShareCounter) {
    ShrdPtr<MultipleDerived> derived(new MultipleDerived());
    ShrdPtr<FirstBase> first(derived);
    ShrdPtr<SecondBase> second(derived);
    EXPECT_EQ(derived.use_count(), 3);
    EXPECT_EQ(first.use_count(), 3);
    EXPECT_EQ(second.use_count(), 3);
}

TEST(InheritanceTest, ShrdPtrRejectsReverseConversion) {
    EXPECT_FALSE((std::is_constructible_v<ShrdPtr<Dog>, const ShrdPtr<Animal>&>));
    EXPECT_TRUE((std::is_constructible_v<ShrdPtr<Animal>, const ShrdPtr<Dog>&>));
}

TEST(InheritanceTest, UnqPtrRawDerivedPointerPreservesType) {
    UnqPtr<Animal> animal(new Dog());
    EXPECT_EQ(animal->get_sound(), 1);
}

TEST(InheritanceTest, UnqPtrRawDerivedPointerCanMove) {
    UnqPtr<Animal> first(new Dog());
    UnqPtr<Animal> second(std::move(first));
    UnqPtr<Animal> third(std::move(second));
    EXPECT_TRUE(first.is_null());
    EXPECT_TRUE(second.is_null());
    EXPECT_EQ(third->get_sound(), 1);
}

TEST(InheritanceTest, UnqPtrRawPointerMultipleInheritance) {
    UnqPtr<SecondBase> first(new MultipleDerived());
    UnqPtr<SecondBase> second(std::move(first));
    EXPECT_TRUE(first.is_null());
    EXPECT_EQ(second->second_value(), 200);
}

TEST(InheritanceTest, UnqPtrRawPointerMoveAssignment) {
    UnqPtr<Animal> first(new Dog());
    UnqPtr<Animal> second;
    second = std::move(first);
    EXPECT_TRUE(first.is_null());
    EXPECT_EQ(second->get_sound(), 1);
}
