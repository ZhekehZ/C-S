#pragma once

#include <cstddef>
#include <utility>

namespace smart_ptr {

    template <typename T> class weak_ptr;


    template <typename T>
    class shared_ptr {
    public:
        using element_type = T;
        friend class weak_ptr<T>;
        template <typename TT>
        friend void swap(shared_ptr<TT>& lhs, shared_ptr<TT>& rhs);

        explicit shared_ptr(T* ptr = nullptr)
                : counters{new Counters{1, 0}}
                , ptr{ptr}
        {}

        shared_ptr(const shared_ptr& other)
                : counters{other.counters}
                , ptr{other.ptr}
        {
            ++counters->shared;
        }

        explicit shared_ptr(const weak_ptr<T>& weak)
                : counters{weak.counters}
                , ptr{weak.ptr}
        {
            if (!counters) {
                counters = new Counters{0, 0};
                ptr = nullptr;
            }
            ++counters->shared;
        }

        shared_ptr &operator=(shared_ptr other) {
            swap(*this, other);
            return *this;
        }

        shared_ptr& operator=(const weak_ptr<T>& weak) {
            shared_ptr other{weak};
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
            if (--counters->shared == 0) {
                delete ptr;
                if (counters->weak == 0) {
                    delete counters;
                }
            }
        }

    private:
        struct Counters {
            size_t shared;
            size_t weak;
        };

        Counters* counters;
        T* ptr;
    };



    template <typename T>
    class weak_ptr {
    public:
        using element_type = T;
        template <typename TT>
        friend void swap(weak_ptr<TT>& lhs, weak_ptr<TT>& rhs);
        friend class shared_ptr<T>;

        weak_ptr()
            : counters{nullptr}
            , ptr{nullptr}
        {}

        weak_ptr(const shared_ptr<T>& shared)
            : counters{shared.counters}
            , ptr{shared.ptr}
        {
            ++counters->weak;
        }

        weak_ptr& operator=(weak_ptr other) {
            swap(*this, other);
            return *this;
        };

        weak_ptr(const weak_ptr& other)
            : counters{other.counters}
            , ptr{other.ptr}
        {
            if (counters) {
                ++counters->weak;
            }
        };

        T* operator->() const {
            return ptr;
        }

        T& operator*() const {
            return *ptr;
        }

        T* get() const {
            return ptr;
        }

        void reset() {
            auto empty = weak_ptr{};
            swap(*this, empty);
        }

        bool expired() const {
            return !counters || counters->shared == 0 || !ptr;
        }

        shared_ptr<T> lock() const {
            if (expired()) {
                return shared_ptr<T>{};
            }
            return shared_ptr<T>{*this};
        }

        ~weak_ptr() {
            if (counters && --counters->weak + counters->shared == 0) {
                delete counters;
            }
        }

    private:
        typename shared_ptr<T>::Counters* counters;
        T* ptr;
    };



    template <typename T>
    void swap(shared_ptr<T>& lhs, shared_ptr<T>& rhs) {
        std::swap(lhs.counters, rhs.counters);
        std::swap(lhs.ptr, rhs.ptr);
    }

    template <typename T>
    void swap(weak_ptr<T>& lhs, weak_ptr<T>& rhs) {
        std::swap(lhs.counters, rhs.counters);
        std::swap(lhs.ptr, rhs.ptr);
    }

}