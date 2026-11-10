#ifndef SHRD_PTR_H
#define SHRD_PTR_H
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
template <class T>
class ShrdPtr {
private:
    template <class U>
    friend class ShrdPtr;
    T* ptr;
    void* owner;
    void (*deleter)(void*);
    std::size_t* reference_count;
    template <class U>
    static void delete_object(void* value) {
        delete static_cast<U*>(value);
    }
    void release_owner() noexcept {
        if (reference_count != nullptr) {
            --(*reference_count);
            if (*reference_count == 0) {
                deleter(owner);
                delete reference_count;
            }
        }
        ptr = nullptr;
        owner = nullptr;
        deleter = nullptr;
        reference_count = nullptr;
    }
public:
    ShrdPtr() noexcept
        : ptr(nullptr), owner(nullptr), deleter(nullptr), reference_count(nullptr) {}
    template <class U = T, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
    explicit ShrdPtr(U* value)
        : ptr(nullptr), owner(nullptr), deleter(nullptr), reference_count(nullptr) {
        if (value != nullptr) {
            try {
                reference_count = new std::size_t(1);
            } catch (...) {
                delete value;
                throw;
            }
            ptr = value;
            owner = value;
            deleter = &delete_object<U>;
        }
    }
    ShrdPtr(const ShrdPtr& other) noexcept
        : ptr(other.ptr), owner(other.owner),
          deleter(other.deleter), reference_count(other.reference_count) {
        if (reference_count != nullptr) {
            ++(*reference_count);
        }
    }
    template <class U, std::enable_if_t<
        std::is_convertible_v<U*, T*> && !std::is_same_v<U, T>, int> = 0>
    ShrdPtr(const ShrdPtr<U>& other) noexcept
        : ptr(other.ptr), owner(other.owner),
          deleter(other.deleter), reference_count(other.reference_count) {
        if (reference_count != nullptr) {
            ++(*reference_count);
        }
    }
    ShrdPtr(ShrdPtr&& other) noexcept
        : ptr(other.ptr), owner(other.owner),
          deleter(other.deleter), reference_count(other.reference_count) {
        other.ptr = nullptr;
        other.owner = nullptr;
        other.deleter = nullptr;
        other.reference_count = nullptr;
    }
    template <class U, std::enable_if_t<
        std::is_convertible_v<U*, T*> && !std::is_same_v<U, T>, int> = 0>
    ShrdPtr(ShrdPtr<U>&& other) noexcept
        : ptr(other.ptr), owner(other.owner),
          deleter(other.deleter), reference_count(other.reference_count) {
        other.ptr = nullptr;
        other.owner = nullptr;
        other.deleter = nullptr;
        other.reference_count = nullptr;
    }
    ~ShrdPtr() {
        release_owner();
    }
    ShrdPtr& operator=(const ShrdPtr& other) noexcept {
        if (this != &other) {
            ShrdPtr temp(other);
            swap(temp);
        }
        return *this;
    }
    template <class U, std::enable_if_t<
        std::is_convertible_v<U*, T*> && !std::is_same_v<U, T>, int> = 0>
    ShrdPtr& operator=(const ShrdPtr<U>& other) noexcept {
        ShrdPtr temp(other);
        swap(temp);
        return *this;
    }
    ShrdPtr& operator=(ShrdPtr&& other) noexcept {
        if (this != &other) {
            release_owner();
            ptr = other.ptr;
            owner = other.owner;
            deleter = other.deleter;
            reference_count = other.reference_count;
            other.ptr = nullptr;
            other.owner = nullptr;
            other.deleter = nullptr;
            other.reference_count = nullptr;
        }
        return *this;
    }
    template <class U, std::enable_if_t<
        std::is_convertible_v<U*, T*> && !std::is_same_v<U, T>, int> = 0>
    ShrdPtr& operator=(ShrdPtr<U>&& other) noexcept {
        release_owner();
        ptr = other.ptr;
        owner = other.owner;
        deleter = other.deleter;
        reference_count = other.reference_count;
        other.ptr = nullptr;
        other.owner = nullptr;
        other.deleter = nullptr;
        other.reference_count = nullptr;
        return *this;
    }
    void swap(ShrdPtr& other) noexcept {
        std::swap(ptr, other.ptr);
        std::swap(owner, other.owner);
        std::swap(deleter, other.deleter);
        std::swap(reference_count, other.reference_count);
    }
    T& operator*() const {
        if (ptr == nullptr) {
            throw std::runtime_error("Cannot dereference null pointer");
        }
        return *ptr;
    }
    T* operator->() const {
        if (ptr == nullptr) {
            throw std::runtime_error("Cannot access null pointer");
        }
        return ptr;
    }
    T* get() const noexcept {
        return ptr;
    }
    std::size_t use_count() const noexcept {
        return reference_count == nullptr ? 0 : *reference_count;
    }
    bool is_null() const noexcept {
        return ptr == nullptr;
    }
    explicit operator bool() const noexcept {
        return ptr != nullptr;
    }
    void reset() noexcept {
        release_owner();
    }
    void reset(T* value) {
        if (ptr != value) {
            ShrdPtr temp(value);
            swap(temp);
        }
    }
};
template <class T>
class ShrdPtr<T[]> {
private:
    T* ptr;
    std::size_t* reference_count;
    void release_owner() noexcept {
        if (reference_count != nullptr) {
            --(*reference_count);
            if (*reference_count == 0) {
                delete[] ptr;
                delete reference_count;
            }
        }
        ptr = nullptr;
        reference_count = nullptr;
    }
public:
    ShrdPtr() noexcept : ptr(nullptr), reference_count(nullptr) {}
    explicit ShrdPtr(T* value) : ptr(nullptr), reference_count(nullptr) {
        if (value != nullptr) {
            try {
                reference_count = new std::size_t(1);
            } catch (...) {
                delete[] value;
                throw;
            }
            ptr = value;
        }
    }
    ShrdPtr(const ShrdPtr& other) noexcept
        : ptr(other.ptr), reference_count(other.reference_count) {
        if (reference_count != nullptr) {
            ++(*reference_count);
        }
    }
    ShrdPtr(ShrdPtr&& other) noexcept
        : ptr(other.ptr), reference_count(other.reference_count) {
        other.ptr = nullptr;
        other.reference_count = nullptr;
    }
    ~ShrdPtr() {
        release_owner();
    }
    ShrdPtr& operator=(const ShrdPtr& other) noexcept {
        if (this != &other) {
            ShrdPtr temp(other);
            swap(temp);
        }
        return *this;
    }
    ShrdPtr& operator=(ShrdPtr&& other) noexcept {
        if (this != &other) {
            release_owner();
            ptr = other.ptr;
            reference_count = other.reference_count;
            other.ptr = nullptr;
            other.reference_count = nullptr;
        }
        return *this;
    }
    void swap(ShrdPtr& other) noexcept {
        std::swap(ptr, other.ptr);
        std::swap(reference_count, other.reference_count);
    }
    T& operator[](std::size_t index) const {
        if (ptr == nullptr) {
            throw std::runtime_error("Cannot access null array");
        }
        return ptr[index];
    }
    T* get() const noexcept {
        return ptr;
    }
    std::size_t use_count() const noexcept {
        return reference_count == nullptr ? 0 : *reference_count;
    }
    bool is_null() const noexcept {
        return ptr == nullptr;
    }
    explicit operator bool() const noexcept {
        return ptr != nullptr;
    }
    void reset() noexcept {
        release_owner();
    }
    void reset(T* value) {
        if (ptr != value) {
            ShrdPtr temp(value);
            swap(temp);
        }
    }
};
#endif
