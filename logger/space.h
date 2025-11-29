#pragma once

#include <ratio>

namespace logger {

template <typename Rep, typename Capacity = std::ratio<1>>
class space;

template <typename ToSpace, typename Rep, typename Capacity>
constexpr ToSpace space_cast(const space<Rep, Capacity>& s) {
    using to_ratio = std::ratio_divide<Capacity, typename ToSpace::period>;
    return ToSpace(s.count() * to_ratio::num / to_ratio::den);
}

/// @brief 基于 std::chrono 设计的存储容量类
/// @tparam Rep 数值类型
/// @tparam Capacity 单位大小
template <typename Rep, typename Capacity>
class space {
public:
    using rep = Rep;
    using period = Capacity;

    constexpr space() : rep_() {};
    ~space() = default;

    template <typename Rep2>
    explicit constexpr space(Rep2 r) : rep_(r) {};

    template <typename Rep2, typename Capacity2>
    constexpr space(const space<Rep2, Capacity2>& other) : rep_(space_cast<space>(other).count()) {};

    // interface
    constexpr Rep count() const {
        return rep_;
    };

    // operator func:+、-、++、--、+=、-=、*=、/=、%=
    constexpr space operator+() const {
        return *this;
    };
    constexpr space operator-() const {
        return space(-count());
    };
    space& operator++() {
        rep_++;
        return *this;
    };
    space operator++(int) {
        space tmp(*this);
        rep_++;
        return tmp;
    };
    space& operator--() {
        rep_--;
        return *this;
    };
    space operator--(int) {
        space tmp(*this);
        rep_--;
        return tmp;
    };
    space& operator+=(const space& other) {
        rep_ += other.count();
        return *this;
    };
    space& operator-=(const space& other) {
        rep_ -= other.count();
        return *this;
    };
    space& operator*=(const Rep& rep) {
        rep_ *= rep;
        return *this;
    };
    space& operator/=(const Rep& rep) {
        rep_ /= rep;
        return *this;
    };
    space& operator%=(const Rep& rep) {
        rep_ %= rep;
        return *this;
    };

private:
    Rep rep_;    

};  // class space

template <typename Rep, typename Capacity>
constexpr space<Rep, Capacity> operator+(const space<Rep, Capacity>& s1, const space<Rep, Capacity>& s2) {
    return space<Rep, Capacity>(s1.count() + s2.count());
}

template <typename Rep, typename Capacity>
constexpr space<Rep, Capacity> operator-(const space<Rep, Capacity>& s1, const space<Rep, Capacity>& s2) {
    return space<Rep, Capacity>(s1.count() - s2.count());
}

using B  = space<size_t>;
using KB = space<size_t, std::ratio<1024L, 1>>;
using MB = space<size_t, std::ratio<1024L * 1024, 1>>;
using GB = space<size_t, std::ratio<1024L * 1024 * 1024, 1>>;
using TB = space<size_t, std::ratio<1024L * 1024 * 1024 * 1024, 1>>;

}   // namespace logger