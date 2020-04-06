#pragma once

#include <stdexcept>

namespace smart_ptr {

    template <typename T>
    class scoped_ptr {
    public:
        using element_type = T;

        explicit scoped_ptr(T* ptr = nullptr) : ptr(ptr) {}

        scoped_ptr(const scoped_ptr&) = delete;
        scoped_ptr& operator=(const scoped_ptr&) = delete;

        T& operator*() const {
            return *ptr;
        }

        T* operator->() const {
            return ptr;
        }

        explicit operator bool() const {
            return ptr != nullptr;
        }

        void reset(T* other = nullptr) {
            delete ptr;
            ptr = other;
        }

        T* release() {
            T* res = ptr;
            ptr = nullptr;
            return res;
        }

        T* get() const {
            return ptr;
        }

        ~scoped_ptr() {
            delete ptr;
        }

    private:
        T* ptr;
    };

}

