#pragma once

#include <utility>
#include <memory>

namespace utils {

    class whatever {
    public:
        whatever() = default;

        template<class T>
        whatever(const T& obj)
            : object_{std::make_unique<hidden<typename std::decay<T>::type>>(obj)}
        {}

        void swap(whatever& other) {
            std::swap(object_, other.object_);
        }

        whatever& operator=(whatever other) {
            swap(other);
            return *this;
        }

        whatever(const whatever& other) : object_{nullptr} {
            if (!other.empty()) {
                object_ = other.object_->copy();
            }
        }

        void clear() {
            object_.reset();
        }

        template<typename T> friend const T* whatever_cast(const whatever* we);

        bool empty() const {
            return object_ == nullptr;
        }

    private:

        struct hidden_base {
            virtual std::unique_ptr<hidden_base> copy() const = 0;
            virtual const std::type_info& get_type() const = 0;
            virtual ~hidden_base() = default;
        };


        template <class T>
        struct hidden : public hidden_base {
            explicit hidden(const T& obj) : obj_(obj) {}

            std::unique_ptr<hidden_base> copy() const override {
                return std::unique_ptr<hidden_base>{new hidden<T>{obj_}};
            }

            const std::type_info& get_type() const override {
                return typeid(obj_);
            }

            T obj_;
        };

        std::unique_ptr<hidden_base> object_;
    };


    inline void swap(whatever& lhs, whatever& rhs) {
        lhs.swap(rhs);
    }

    struct bad_whatever_cast : public std::runtime_error {
        bad_whatever_cast() : runtime_error("bad_whatever_cast") {}
    };


    template<typename T>
    const T* whatever_cast(const whatever* we) {
        using RealType = typename std::decay<T>::type;
        using HiddenType = typename whatever::hidden<RealType>;
        if (!we || !we->object_ || we->object_->get_type() != typeid(RealType)) {
            return nullptr;
        }
        return &static_cast<HiddenType *>(we->object_.get())->obj_;
    }

    template<typename T>
    T* whatever_cast(whatever* we) {
        const whatever* we_const = we;
        return const_cast<T*>(whatever_cast<T>(we_const));
    }

    template<typename T>
    T whatever_cast(whatever &we) {
        using RealType = typename std::decay<T>::type;
        if (auto tmp = whatever_cast<RealType>(&we)) {
            return *tmp;
        }
        throw bad_whatever_cast();
    }

    template<typename T>
    T whatever_cast(const whatever &we) {
        using RealType = typename std::decay<T>::type;
        if (auto tmp = whatever_cast<RealType>(&we)) {
            return *tmp;
        }
        throw bad_whatever_cast();
    }

}
