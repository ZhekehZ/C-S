#pragma once

#include <cstddef>
#include <utility>

namespace smart_ptr {

    template <typename T>
    class shared_ptr {
    public:
        using element_type = T;

        explicit shared_ptr(T* ptr = nullptr)
            : shared_counter{new size_t{1}}
            , ptr{ptr}
        {}

        template <typename TT>
        friend void swap(shared_ptr<TT>& lhs, shared_ptr<TT>& rhs);

        shared_ptr(const shared_ptr& other)
            : shared_counter{other.shared_counter}
            , ptr{other.ptr}
        {
            ++*shared_counter;
        }

        shared_ptr &operator=(shared_ptr other) {
            swap(*this, other);
            return *this;
        }

        T* operator->() const {
            return ptr;
        }

        T& operator*() const {
            return *ptr;
        }

        T* get() const {
            return ptr;
        }

        void reset(T* other = nullptr) {
            shared_ptr tmp(other);
            swap(*this, tmp);
        }

        explicit operator bool() const {
            return ptr != nullptr;
        }

        ~shared_ptr() {
            --*shared_counter;
            if (*shared_counter == 0) {
                delete ptr;
                delete shared_counter;
            }
        }

    private:
        size_t *shared_counter;
        T* ptr;
    };


    template <typename T>
    void swap(shared_ptr<T>& lhs, shared_ptr<T>& rhs) {
        std::swap(lhs.shared_counter, rhs.shared_counter);
        std::swap(lhs.ptr, rhs.ptr);
    }
}