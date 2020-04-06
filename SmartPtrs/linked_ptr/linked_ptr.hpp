#pragma once

#include <utility>

namespace smart_ptr {

    namespace details {

        struct linked_list {
            linked_list *next_ = nullptr;
            linked_list *prev_ = nullptr;

            linked_list() = default;

            linked_list(linked_list &other)
                    : next_(&other), prev_(other.prev_)
            {
                reconnect();
            }

            ~linked_list() {
                if (next_) {
                    next_->prev_ = prev_;
                }
                if (prev_) {
                    prev_->next_ = next_;
                }
            }

            void reconnect() {
                if (prev_) {
                    prev_->next_ = this;
                }
                if (next_) {
                    next_->prev_ = this;
                }
            }

            bool unique() {
                return !next_ && !prev_;
            }

            void swap(linked_list &other) {
                std::swap(next_, other.next_);
                std::swap(prev_, other.prev_);
                reconnect();
                other.reconnect();
            }
        };

    }


    template<typename T>
    class linked_ptr {
    public:
        using element_type = T;

        explicit linked_ptr(T *ptr = nullptr) : ptr_(ptr) {}

        linked_ptr(const linked_ptr &other) : ptr_(other.ptr_), links_(other.links_) {}

        template<typename U>
        explicit linked_ptr(U *ptr) : ptr_(ptr) {}

        template<typename U>
        linked_ptr(const linked_ptr<U> &other)
                : ptr_(other.ptr_), links_(other.links_) {}

        void swap(linked_ptr &other) {
            if (other != *this) {
                links_.swap(other.links_);
                std::swap(ptr_, other.ptr_);
            }
        }

        template<typename U>
        linked_ptr &operator=(linked_ptr<U> other) {
            if (other != *this) {
                links_.swap(other.links_);
                ptr_ = other.ptr_;
            }
            return *this;
        }

        linked_ptr &operator=(linked_ptr other) {
            swap(other);
            return *this;
        }

        T *get() const {
            return ptr_;
        }

        bool unique() const {
            return links_.unique() && ptr_;
        }

        T &operator*() const {
            return *ptr_;
        }

        T *operator->() const {
            return ptr_;
        }

        explicit operator bool() const {
            return ptr_;
        }

        void reset() {
            swap({});
        }

        template<typename U>
        void reset(U *ptr = nullptr) {
            linked_ptr lptr(ptr);
            swap(lptr);
        }

        ~linked_ptr() {
            static_assert(0 < sizeof(T), "can't delete an incomplete type");

            if (unique()) {
                delete ptr_;
            }
        }

        template<class> friend class linked_ptr;

    private:
        T *ptr_;
        mutable details::linked_list links_;
    };


    template<typename T, typename U>
    inline bool operator<(const linked_ptr<T> &lhs, const linked_ptr<U> &rhs) {
        return lhs.get() < rhs.get();
    }

    template<typename T, typename U>
    inline bool operator>(const linked_ptr<T> &lhs, const linked_ptr<U> &rhs) {
        return lhs.get() > rhs.get();
    }

    template<typename T, typename U>
    inline bool operator<=(const linked_ptr<T> &lhs, const linked_ptr<U> &rhs) {
        return lhs.get() <= rhs.get();
    }

    template<typename T, typename U>
    inline bool operator>=(const linked_ptr<T> &lhs, const linked_ptr<U> &rhs) {
        return lhs.get() >= rhs.get();
    }

    template<typename T, typename U>
    inline bool operator==(const linked_ptr<T> &lhs, const linked_ptr<U> &rhs) {
        return lhs.get() == rhs.get();
    }

    template<typename T, typename U>
    inline bool operator!=(const linked_ptr<T> &lhs, const linked_ptr<U> &rhs) {
        return lhs.get() != rhs.get();
    }

    template<typename T, typename U>
    inline void swap(linked_ptr<T> &lhs, linked_ptr<U> &rhs) {
        lhs.swap(rhs);
    }

}

