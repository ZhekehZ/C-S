#pragma once

#include <utility>
#include <memory>
#include <unordered_map>

namespace utils {

    class whatever {
    public:
        using dict_t = std::unordered_map<std::string, whatever>;
        using ini_list = std::initializer_list<std::pair<const std::string, whatever>>;

        whatever() = default;

        whatever(const ini_list& data): whatever(dict_t(data)) {}

        template<class T>
        whatever(T&& obj)
                : object_{std::make_unique<hidden<typename std::decay<T>::type>>(std::forward<T>(obj))}
        {}

        whatever(const char* str) : whatever(std::string(str)) {}

        void swap(whatever& other) {
            std::swap(object_, other.object_);
        }

        whatever(const whatever& other) : object_{nullptr} {
            if (!other.empty()) {
                object_ = other.object_->copy();
            }
        }

        whatever(whatever&& other) noexcept : object_{nullptr} {
            swap(other);
        }

        whatever& operator=(whatever other) {
            swap(other);
            return *this;
        }

        bool operator==(const whatever &rhs) const {
            return *object_ == *rhs.object_;
        }

        bool operator!=(const whatever &rhs) const {
            return !(rhs == *this);
        }

        void clear() {
            object_.reset();
        }

        template<typename T> friend const T* whatever_cast(const whatever* we);

        bool empty() const {
            return object_ == nullptr;
        }

        std::pair<bool, int> as_int() const {
            return object_->as_int();
        }

        std::pair<bool, double> as_double() const {
            return object_->as_double();
        }

        std::pair<bool, bool> as_bool() const {
            return object_->as_bool();
        }

        std::pair<bool, std::string> as_string() const {
            return object_->as_string();
        }

    private:

        struct hidden_base {
            virtual std::unique_ptr<hidden_base> copy() const = 0;
            virtual const std::type_info& get_type() const = 0;
            virtual bool operator==(const hidden_base& other) const = 0;
            virtual ~hidden_base() = default;

            virtual std::pair<bool, int>         as_int()     const = 0;
            virtual std::pair<bool, double>      as_double()  const = 0;
            virtual std::pair<bool, bool>        as_bool()    const = 0;
            virtual std::pair<bool, std::string> as_string () const = 0;
        };


        template <class T>
        struct hidden : public hidden_base {
            explicit hidden(T&& obj) : obj_(std::forward<T>(obj)) {}
            explicit hidden(const T& obj) : obj_(obj) {}

            std::unique_ptr<hidden_base> copy() const override {
                return std::unique_ptr<hidden_base>{new hidden<T>{obj_}};
            }

            bool operator==(const hidden_base &other) const override {
                if (other.get_type() != get_type()) {
                    return false;
                }
                return dynamic_cast<const hidden&>(other).obj_ == obj_;
            }

            std::pair<bool, int> as_int() const override {
                if constexpr (IS_INTEGRAL) {
                    return {true, static_cast<int>(obj_) };
                }
                return {false, 0};
            }

            std::pair<bool, double> as_double() const override {
                if constexpr (IS_FLOATING) {
                    return {true, static_cast<double>(obj_) };
                }
                return {false, 0};
            }

            std::pair<bool, bool> as_bool() const override {
                if constexpr (IS_BOOL) {
                    return {true, static_cast<bool>(obj_) };
                }
                return {false, 0};
            }

            std::pair<bool, std::string> as_string() const override {
                if constexpr (IS_STRING) {
                    return {true, static_cast<std::string>(obj_) };
                }
                return {false, nullptr};
            }

            const std::type_info& get_type() const override {
                return typeid(obj_);
            }

            static constexpr bool IS_INTEGRAL = std::is_integral_v<T> && !std::is_same_v<T, bool>;
            static constexpr bool IS_FLOATING = std::is_floating_point_v<T>;
            static constexpr bool IS_BOOL     = std::is_same_v<T, bool>;
            static constexpr bool IS_STRING   = std::is_same_v<T, std::string>;
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