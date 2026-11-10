#ifndef UNQ_PTR_H
#define UNQ_PTR_H
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>

template <class T>
class UnqPtr {
private:
    template <class U>
    friend class UnqPtr;
    T* ptr;
    void* owner;
    void (*deleter)(void*);
    template <class U>
    static void delete_object(void* value) {
        delete static_cast<U*>(value);
    }
    void destroy() noexcept {
        if (owner != nullptr) {
            deleter(owner);
        }
        ptr = nullptr;
        owner = nullptr;
        deleter = nullptr;
    }
public:
    UnqPtr() noexcept : ptr(nullptr), owner(nullptr), deleter(nullptr) {}
    template <class U = T, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
    explicit UnqPtr(U* value) noexcept
        : ptr(value), owner(value),
          deleter(value != nullptr ? &delete_object<U> : nullptr) {}
    UnqPtr(const UnqPtr&) = delete;
    UnqPtr& operator=(const UnqPtr&) = delete;
    template <class U>
    UnqPtr(const UnqPtr<U>&) = delete;
    template <class U>
    UnqPtr& operator=(const UnqPtr<U>&) = delete;
    UnqPtr(UnqPtr&& other) noexcept
        : ptr(other.ptr), owner(other.owner), deleter(other.deleter) {
        other.ptr = nullptr;
        other.owner = nullptr;
        other.deleter = nullptr;
    }
    template <class U, std::enable_if_t<
        std::is_convertible_v<U*, T*> && !std::is_same_v<U, T>, int> = 0>
    UnqPtr(UnqPtr<U>&& other) noexcept
        : ptr(other.ptr), owner(other.owner), deleter(other.deleter) {
        other.ptr = nullptr;
        other.owner = nullptr;
        other.deleter = nullptr;
    }
    ~UnqPtr() {
        destroy();
    }
    UnqPtr& operator=(UnqPtr&& other) noexcept {
        if (this != &other) {
            destroy();
            ptr = other.ptr;
            owner = other.owner;
            deleter = other.deleter;
            other.ptr = nullptr;
            other.owner = nullptr;
            other.deleter = nullptr;
        }
        return *this;
    }
    template <class U, std::enable_if_t<
        std::is_convertible_v<U*, T*> && !std::is_same_v<U, T>, int> = 0>
    UnqPtr& operator=(UnqPtr<U>&& other) noexcept {
        destroy();
        ptr = other.ptr;
        owner = other.owner;
        deleter = other.deleter;
        other.ptr = nullptr;
        other.owner = nullptr;
        other.deleter = nullptr;
        return *this;
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
    bool is_null() const noexcept {
        return ptr == nullptr;
    }
    explicit operator bool() const noexcept {
        return ptr != nullptr;
    }
    T* release() noexcept {
        T* result = ptr;
        ptr = nullptr;
        owner = nullptr;
        deleter = nullptr;
        return result;
    }
    void reset() noexcept {
        destroy();
    }
    template <class U = T, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
    void reset(U* value) noexcept {
        if (ptr != value) {
            destroy();
            ptr = value;
            owner = value;
            deleter = value != nullptr ? &delete_object<U> : nullptr;
        }
    }
};

template <class T>
class UnqPtr<T[]> {
private:
    T* ptr;
public:
    UnqPtr() noexcept : ptr(nullptr) {}
    explicit UnqPtr(T* value) noexcept : ptr(value) {}
    UnqPtr(const UnqPtr&) = delete;
    UnqPtr& operator=(const UnqPtr&) = delete;
    UnqPtr(UnqPtr&& other) noexcept : ptr(other.ptr) {
        other.ptr = nullptr;
    }
    UnqPtr& operator=(UnqPtr&& other) noexcept {
        if (this != &other) {
            delete[] ptr;
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }
    ~UnqPtr() {
        delete[] ptr;
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
    bool is_null() const noexcept {
        return ptr == nullptr;
    }
    explicit operator bool() const noexcept {
        return ptr != nullptr;
    }
    T* release() noexcept {
        T* result = ptr;
        ptr = nullptr;
        return result;
    }
    void reset(T* value = nullptr) noexcept {
        if (ptr != value) {
            delete[] ptr;
            ptr = value;
        }
    }
};
#endif
