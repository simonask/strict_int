#pragma once
#ifndef STRICT_INT_HPP
#define STRICT_INT_HPP

#include <cstddef>
#include <cstdint>
#include <stdexcept> // std::overflow_error
#include <limits> // std::numeric_limits
#include <cassert>

namespace strict_int {
    using Bits = std::size_t;

    enum class Overflow {
        Wrap,
        Throw,
        Clamp
    };

    enum class Sign {
        Signed,
        Unsigned
    };

    template <Sign sign, Bits bits> struct int_repr;
    template <Sign sign, Bits bits>
    using int_repr_t = typename int_repr<sign, bits>::type;

    template <> struct int_repr<Sign::Signed,    8> { using type = std::int_least8_t;   };
    template <> struct int_repr<Sign::Signed,   16> { using type = std::int_least16_t;  };
    template <> struct int_repr<Sign::Signed,   32> { using type = std::int_least32_t;  };
    template <> struct int_repr<Sign::Signed,   64> { using type = std::int_least64_t;  };
    template <> struct int_repr<Sign::Unsigned,  8> { using type = std::uint_least8_t;  };
    template <> struct int_repr<Sign::Unsigned, 16> { using type = std::uint_least16_t; };
    template <> struct int_repr<Sign::Unsigned, 32> { using type = std::uint_least32_t; };
    template <> struct int_repr<Sign::Unsigned, 64> { using type = std::uint_least64_t; };

    template <class T, Sign _sign, Bits _bits, Overflow _overflow>
    struct integer_ops;

    template <class T, Sign _sign, Bits _bits>
    struct integer_ops<T, _sign, _bits, Overflow::Wrap> {
        using repr = int_repr_t<_sign, _bits>;
        using unsigned_repr = int_repr_t<Sign::Unsigned, _bits>;

        static constexpr repr add(repr a, repr b) noexcept {
            return repr(unsigned_repr(a) + unsigned_repr(b));
        }

        static constexpr repr sub(repr a, repr b) noexcept {
            return repr(unsigned_repr(a) - unsigned_repr(b));
        }

        static constexpr repr mul(repr a, repr b) noexcept {
            return a*b; // TODO
        }

        static constexpr repr div(repr a, repr b) noexcept {
            return a/b; // TODO
        }

        static constexpr repr shl(repr a, Bits shift) noexcept {
            return a << shift;
        }

        static constexpr repr shr(repr a, Bits shift) noexcept {
            return a >> shift;
        }
    };

    enum class Error {
        None = 0,
        Underflow,
        Overflow,
        DivisionByZero,
    };

    template <class T, class L = std::numeric_limits<T>>
    Error try_add(T& a, T b) {
        if (b < 0) {
            if (L::min() - b > a)
                return Error::Underflow;
        } else {
            if (L::max() - b < a)
                return Error::Overflow;
        }
        a += b;
        return Error::None;
    }

    template <class T, class L = std::numeric_limits<T>>
    Error try_sub(T& a, T b) {
        if (b < 0) {
            if (L::max() + b < a)
                return Error::Overflow;
        } else {
            if (L::min() + b > a)
                return Error::Underflow;
        }
        a -= b;
        return Error::None;
    }

    template <class T, class L = std::numeric_limits<T>>
    Error try_mul(T& a, T b) {
        if (b == 0) {
            a = 0; // avoid division by zero
            return Error::None;
        }
        if (L::max() / b < a)
            return Error::Overflow;
        a *= b;
        return Error::None;
    }

    template <class T, class L = std::numeric_limits<T>>
    Error try_div(T& a, T b) {
        if (b == 0)
            return Error::DivisionByZero;
        a /= b;
        return Error::None;
    }

    template <class T, class L = std::numeric_limits<T>>
    Error try_shl(T& a, Bits b) {
        // TODO: Implement
        a <<= b;
        return Error::None;
    }

    template <class T, class L = std::numeric_limits<T>>
    Error try_shr(T& a, Bits b) {
        a >>= b;
        return Error::None;
    }

    template <class T, Sign _sign, Bits _bits, Overflow _overflow>
    struct strict_int;

    template <Bits, Sign> struct portable_limits;

    template <Bits bits>
    struct portable_limits<bits, Sign::Unsigned> {
        using limit_type = uint_fast64_t;

        static constexpr limit_type min() { return 0; }
        static constexpr limit_type lowest() { return 0; }
        static constexpr limit_type max() { return (1ULL << (bits-1)) - 1; }
    };

    template <Bits bits>
    struct portable_limits<bits, Sign::Signed> {
        using limit_type = int_fast64_t;

        // TODO: The following rely on 2-complement.
        static constexpr limit_type min() { return (1ULL << (bits-1)) ^ ~0LL; }
        static constexpr limit_type lower() { return min(); }
        static constexpr limit_type max() { return ~(1LL << (bits-1)); }
    };


    template <class T, Sign _sign, Bits _bits>
    struct integer_ops<T, _sign, _bits, Overflow::Throw> {
        using repr = int_repr_t<_sign, _bits>;
        using limits = portable_limits<_bits, _sign>;

        static repr add(repr a, repr b) {
            switch (try_add<repr, limits>(a, b)) {
                case Error::None: return a;
                case Error::Underflow: throw std::underflow_error("add");
                case Error::Overflow: throw std::overflow_error("add");
                case Error::DivisionByZero: assert(false);
            }
        }

        static repr sub(repr a, repr b) {
            switch (try_sub<repr, limits>(a, b)) {
                case Error::None: return a;
                case Error::Underflow: throw std::underflow_error("sub");
                case Error::Overflow: throw std::overflow_error("sub");
                case Error::DivisionByZero: assert(false);
            }
        }

        static repr mul(repr a, repr b) {
            switch (try_mul<repr, limits>(a, b)) {
                case Error::None: return a;
                case Error::Underflow: throw std::underflow_error("mul");
                case Error::Overflow: throw std::overflow_error("mul");
                case Error::DivisionByZero: assert(false);
            }
        }

        static repr div(repr a, repr b) {
            switch (try_div<repr, limits>(a, b)) {
                case Error::None: return a;
                case Error::Underflow:
                case Error::Overflow: assert(false);
                case Error::DivisionByZero: throw std::underflow_error("division by zero"); // TODO: Consider exception type
            }
        }

        static repr shl(repr a, Bits b) {
            switch (try_shl<repr, limits>(a, b)) {
                case Error::None: return a;
                case Error::Underflow: throw std::underflow_error("shl");
                case Error::Overflow: throw std::overflow_error("shl");
                case Error::DivisionByZero: assert(false);
            }
        }

        static repr shr(repr a, Bits b) {
            switch (try_shr<repr, limits>(a, b)) {
                case Error::None: return a;
                case Error::Underflow: throw std::underflow_error("shr");
                case Error::Overflow: throw std::overflow_error("shr");
                case Error::DivisionByZero: assert(false);
            }
        }
    };

    template <class To, class From> To int_cast(From) noexcept(To::overflow != Overflow::Throw);

    template <class T, Sign _sign, Bits _bits, Overflow _overflow>
    struct strict_int {
    protected:
        using ops = integer_ops<T, _sign, _bits, _overflow>;
        using repr = int_repr_t<_sign, _bits>;
    public:
        static const bool is_signed = (_sign == Sign::Signed);
        static const Bits bits = _bits;
        static const Overflow overflow = _overflow;
        static const bool is_noexcept = (_overflow != Overflow::Throw);


        constexpr strict_int() noexcept {}
        constexpr explicit strict_int(repr n) noexcept : n_(n) {}
        constexpr strict_int& operator=(const strict_int& other) noexcept = default;
        constexpr bool operator==(const strict_int& other) const noexcept { return n_ == other.n_; }

        T operator+(T other) const noexcept(is_noexcept) { return T{ops::add(n_, other.n_)}; }
        T operator-(T other) const noexcept(is_noexcept) { return T{ops::sub(n_, other.n_)}; }
        T operator*(T other) const noexcept(is_noexcept) { return T{ops::mul(n_, other.n_)}; }
        T operator/(T other) const noexcept(is_noexcept) { return T{ops::div(n_, other.n_)}; }
        T operator<<(Bits n) const noexcept(is_noexcept) { return T{ops::shl(n_, n)}; }
        T operator>>(Bits n) const noexcept(is_noexcept) { return T{ops::shr(n_, n)}; }

        T operator&(T other) const noexcept { return T{n_ & other.n_}; }
        T operator|(T other) const noexcept { return T{n_ | other.n_}; }
        T operator^(T other) const noexcept { return T{n_ ^ other.n_}; }
        T operator~() const noexcept { return T{~n_}; }
        T operator!() const noexcept { return T{!n_}; }
        T operator-() const noexcept(is_noexcept) { return T{-n_}; }

        static constexpr T max() noexcept { return T{portable_limits<_bits, _sign>::max()}; }
        static constexpr T min() noexcept { return T{portable_limits<_bits, _sign>::min()}; }
    protected:
        repr n_;
        template <class To, class From> friend To int_cast(From) noexcept(To::overflow != Overflow::Throw);
    };

    template <class To, class From>
    To int_cast(From from) noexcept(To::overflow != Overflow::Throw) {
        if constexpr (To::overflow == Overflow::Throw) {
            if (from.n_ > To::max().n_)
                throw std::overflow_error("int_cast");
            if (from.n_ < To::min().n_)
                throw std::underflow_error("int_cast");
        } else if constexpr (To::overflow == Overflow::Clamp) {
            if (from.n_ > To::max().n_)
                return To::max();
            if (from.n_ < To::min().n_)
                return To::min();
        }

        return To(from.n_);
    }

    template <class T, Bits bits>
    using wrapping_int = strict_int<T, Sign::Signed, bits, Overflow::Wrap>;
    template <class T, Bits bits>
    using wrapping_uint = strict_int<T, Sign::Unsigned, bits, Overflow::Wrap>;
    template <class T, Bits bits>
    using throwing_int = strict_int<T, Sign::Signed, bits, Overflow::Throw>;
    template <class T, Bits bits>
    using throwing_uint = strict_int<T, Sign::Unsigned, bits, Overflow::Throw>;

    struct i8 : wrapping_int<i8, 8> {
        using wrapping_int<i8, 8>::wrapping_int;
    };

    struct i16 : wrapping_int<i16, 16> {
        using wrapping_int<i16, 16>::wrapping_int;
    };

    struct i32 : wrapping_int<i32, 32> {
        using wrapping_int<i32, 32>::wrapping_int;
    };

    struct i64 : wrapping_int<i64, 64> {
        using wrapping_int<i64, 64>::wrapping_int;
    };

    struct u8 : wrapping_uint<u8, 8> {
        using wrapping_uint<u8, 8>::wrapping_uint;
    };

    struct u16 : wrapping_uint<u16, 16> {
        using wrapping_uint<u16, 16>::wrapping_uint;
    };

    struct u32 : wrapping_uint<u32, 32> {
        using wrapping_uint<u32, 32>::wrapping_uint;
    };

    struct u64 : wrapping_uint<u64, 64> {
        using wrapping_uint<u64, 64>::wrapping_uint;
    };

    static const Bits size_bits = sizeof(std::size_t)*8;

    struct isize : throwing_int<isize, size_bits> {
        using throwing_int<isize, size_bits>::throwing_int;
    };

    struct usize : throwing_uint<usize, size_bits> {
        using throwing_uint<usize, size_bits>::throwing_uint;
    };

    static const Bits pointer_bits = sizeof(void*)*8;

    struct iptr : throwing_int<iptr, pointer_bits> {
        using throwing_int<iptr, pointer_bits>::throwing_int;
    };

    struct uptr : throwing_uint<uptr, pointer_bits> {
        using throwing_uint<uptr, pointer_bits>::throwing_uint;
    };
}

#endif // STRICT_INT_HPP
