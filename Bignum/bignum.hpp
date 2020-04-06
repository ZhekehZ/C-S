#pragma once

#include <vector>
#include <algorithm>
#include <sstream>
#include <cassert>

namespace mp {

    constexpr size_t MAX_SMALL_LENGTH = 20;

    class bignum {
    public:
        bignum() : bigval(nullptr), small(0) {}


        bignum(uint32_t num) : bigval(nullptr), small(num) {}


        ~bignum() {
            delete bigval;
        }


        bignum(const bignum& other)
            : bigval(nullptr)
            , small(other.small)
        {
            if (other.bigval) {
                bigval = new std::vector<uint32_t>(*other.bigval);
            }
        }


        bignum& operator=(const bignum& other) {
            if (this != &other) {
                delete bigval;
                bigval = nullptr;
                small = other.small;
                if (other.bigval) {
                    bigval = new std::vector<uint32_t>(*other.bigval);
                }
            }
            return *this;
        }


        explicit bignum(const std::string& decimals)
            : bignum()
        {
            if (decimals.size() < MAX_SMALL_LENGTH) {
                std::istringstream iss(decimals);
                iss >> small;
            } else {
                bigval = new std::vector<uint32_t>(1, 0);
                for (char decimal : decimals) {
                    *this *= 10u;
                    *this += static_cast<uint32_t>(decimal - '0');
                }
            }
        }


        std::string to_string() const {
            if (!bigval) {
                return std::to_string(small);
            }
            std::string res = {0};
            for (auto it = bigval->rbegin(); it != bigval->rend(); ++it) {
                mul_decimals(res, {6, 9, 2, 7, 6, 9, 4, 9, 2, 4}); // 2^32 = 4294967296
                std::string str = std::to_string(*it);
                std::reverse(str.begin(), str.end());
                for (auto& d : str) {
                    d -= '0';
                }
                sum_decimals(res, str);
            }
            for (auto& d : res) {
                d += '0';
            }
            std::reverse(res.begin(), res.end());
            return res;
        }


        explicit operator uint32_t() const {
            if (!bigval) {
                return small;
            }
            return bigval->front();
        }


        explicit operator bool() const {
            if (!bigval) {
                return small > 0;
            }
            return bigval->size() > 1 || bigval->front() > 0;
        }


        bignum& operator+=(const bignum& rhs) {
            if (!rhs.bigval) {
                return *this += rhs.small;
            }
            if (!bigval) {
                init_big();
            }
            uint32_t rem = 0;
            size_t new_size = std::max(bigval->size(), rhs.bigval->size()) + 1;
            for (size_t i = 0; i < new_size; ++i) {
                uint64_t sum = rem;
                if (i < rhs.bigval->size()) {
                    sum += rhs.bigval->at(i);
                }
                if (i < bigval->size()) {
                    sum += bigval->at(i);
                }
                bigval->at(i) = static_cast<uint32_t>(sum);
                rem = static_cast<uint32_t>(sum >> 32u);
                if (i + 1 == bigval->size()) {
                    bigval->push_back(0);
                }
            }
            while (bigval->size() > 1 && bigval->back() == 0) {
                bigval->pop_back();
            }
            return *this;
        }


        bignum& operator*=(const bignum& rhs) {
            if (!rhs.bigval) {
                return *this *= rhs.small;
            }
            if (!bigval) {
                init_big();
            }
            std::vector<uint64_t> result(bigval->size() + rhs.bigval->size() + 2);
            for (size_t i = 0; i < bigval->size(); ++i) {
                for (size_t j = 0; j < rhs.bigval->size(); ++j) {
                    result[i + j] += static_cast<uint64_t>(bigval->at(i)) * rhs.bigval->at(j);
                }
                uint32_t rem = 0;
                for (uint64_t & j : result) {
                    j += rem;
                    rem = j >> 32u;
                    j = static_cast<uint32_t>(j);
                }
            }
            bigval->resize(result.size());
            for (size_t i = 0; i < bigval->size(); ++i) {
                bigval->at(i) = result[i];
            }
            while (bigval->size() > 1 && bigval->back() == 0) {
                bigval->pop_back();
            }

            return *this;
        }


        bignum& operator*=(uint64_t mul) {
            uint64_t res;
            bool overflow_small = __builtin_umull_overflow(small, mul, &res);
            if (!bigval && !overflow_small) {
                small = res;
                return *this;
            }

            if (!bigval) {
                init_big();
            }

            if (mul > std::numeric_limits<uint32_t>::max()) {
                bignum tmp = *this;
                tmp *= static_cast<uint32_t>(mul);
                *this *= mul >> 32u;
                bigval->push_back(0);
                return *this += tmp;
            }

            uint32_t rem = 0;
            for (auto& digit : *bigval) {
                uint64_t prod = mul;
                prod *= digit;
                prod += rem;
                digit = static_cast<uint32_t>(prod);
                rem = static_cast<uint32_t>(prod >> 32u);
            }
            if (rem > 0) {
                bigval->push_back(rem);
            }
            return *this;
        }


        bignum& operator+=(uint64_t add) {
            if (!bigval && UINT64_MAX - small >= add) {
                small += add;
                return *this;
            }
            if (!bigval) {
                init_big();
            }

            bool overflow = add > std::numeric_limits<uint32_t>::max();
            uint64_t low = static_cast<uint32_t>(add);
            uint32_t back = bigval->back();

            if (overflow) {
                bigval->pop_back();
                if (bigval->empty()) {
                    bigval->push_back(0);
                }
                add >>= 32u;
            }

            uint32_t rem = add;
            for (auto& digit32 : *bigval) {
                uint64_t sum = rem;
                auto& t = bigval;
                (void) t;
                sum += digit32;
                digit32 = static_cast<uint32_t>(sum);
                rem = static_cast<uint32_t>(sum >> 32u);
            }
            if (rem > 0) {
                bigval->push_back(rem);
            }

            if (overflow) {
                bigval->push_back(back);
                *this += low;
            }

            return *this;
        }


    private:
        void init_big() {
            assert(!bigval);
            bigval = new std::vector<uint32_t>{
                static_cast<uint32_t>(small),
                static_cast<uint32_t>(small >> 32u)
            };
        }


        static void mul_decimals(
                std::string& num1,
                const std::string& num2
        ) {
            std::string result(num1.size() + num2.size(), 0);
            for (size_t i = 0; i < num1.size(); ++i) {
                for (size_t j = 0; j < num2.size(); ++j) {
                    result[i + j] += num1[i] *  num2[j];
                }

                for (size_t j = 0; j < result.size() - 1; ++j) {
                    result[j + 1] += result[j] / 10;
                    result[j] = result[j] % 10;
                }
            }
            while (result.size() > 1 && result.back() == 0) {
                result.pop_back();
            }
            num1 = result;
        }


        static void sum_decimals(
                std::string& num1,
                const std::string& num2
        ) {
            for (size_t i = 0; i < num2.size(); ++i) {
                num1[i] += num2[i];
                if (i + 1 == num1.size()) {
                    num1.push_back(0);
                }
            }
            for (size_t j = 0; j < num1.size() - 1; ++j) {
                num1[j + 1] += num1[j] / 10;
                num1[j] = num1[j] % 10;
            }
            while (num1.size() > 1 && num1.back() == 0) {
                num1.pop_back();
            }
        }


        std::vector<uint32_t>* bigval;
        uint64_t small; // small object optimization
    };



    inline bignum operator*(const bignum& lhs, const bignum& rhs) {
        bignum res = lhs;
        res *= rhs;
        return res;
    }



    inline bignum operator+(const bignum& lhs, const bignum& rhs) {
        bignum res = lhs;
        res += rhs;
        return res;
    }



    inline std::ostream& operator<<(std::ostream& out, const bignum& num) {
        return out << num.to_string();
    }



    inline std::istream& operator>>(std::istream& in, bignum& num) {
        std::string s;
        if (in.peek() == '+') {
            in.get();
        }
        in >> s;
        num = bignum(s);
        return in;
    }



    class polynomial {
    public:
        explicit polynomial(const std::string& s) {
            std::vector<std::pair<uint32_t, uint32_t>> polynom;
            size_t start = 0;
            size_t pos = s.find_first_of("+-");
            while (pos != std::string::npos) {
                polynom.push_back(get_monom(s, start));
                start = pos;
                pos = s.find_first_of("+-", pos + 1);
            }
            polynom.push_back(get_monom(s, start));

            std::sort(polynom.begin(), polynom.end());
            coeffs_.assign(polynom.rbegin()->first + 1, 0);
            for (auto& a : polynom) {
                coeffs_[a.first] = a.second;
            }
        }


        uint32_t at(size_t idx) const {
            if (idx < coeffs_.size()) {
                return coeffs_[idx];
            }
            return 0;
        }


        uint32_t& at(size_t idx) {
            if (idx < coeffs_.size()) {
                return coeffs_[idx];
            }
            if (idx >= coeffs_.size()) {
                coeffs_.resize(idx + 1, 0);
            }
            return coeffs_.back();
        }


        template <class T>
        T operator()(const T& x) const{
            T curr = 0;
            for (auto it = coeffs_.rbegin(); it != coeffs_.rend(); ++it) {
                curr *= x;
                curr += *it;
            }
            return curr;
        }


    private:
        static std::pair<uint32_t, uint32_t> get_monom(const std::string& str, size_t& pos) {
            char* next = nullptr;
            uint32_t coeff = std::strtoull(str.c_str() + pos, &next, 0);
            size_t next_index = (next - str.c_str()) + 3;
            uint32_t pow = std::strtoull(str.c_str() + next_index, &next, 0);
            return {pow, coeff};
        }


        std::vector<uint32_t> coeffs_;
    };
}