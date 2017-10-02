#include <gtest/gtest.h>

#include <strict_int/strict_int.hpp>

using namespace strict_int;

struct throwing_i32 : throwing_int<throwing_i32, 32> {
    using throwing_int<throwing_i32, 32>::throwing_int;
};

TEST(StrictInt, wrap_overflow_add) {
    i32 a = i32::max();
    i32 b = a + i32{1};
    ASSERT_EQ(b, i32::min());
}

TEST(StrictInt, detect_overflow_add) {
    isize a = isize::max();
    ASSERT_THROW(a + isize(1), std::overflow_error);
    ASSERT_THROW(isize(1) + a, std::overflow_error);
    isize b = isize::min();
    ASSERT_THROW(b + isize(-1), std::underflow_error);
    ASSERT_THROW(isize(-1) + b, std::underflow_error);
}

TEST(StrictInt, detect_overflow_sub) {
    isize a = isize::max();
    ASSERT_THROW(a - isize(-1), std::overflow_error);
    ASSERT_THROW(isize(-2) - a, std::underflow_error);
    isize b = isize::min();
    ASSERT_THROW(b - isize(1), std::underflow_error);
    ASSERT_THROW(isize(1) - b, std::overflow_error);
}

TEST(StrictInt, detect_division_by_zero) {
    isize a(5);
    ASSERT_THROW(a / isize(0), std::underflow_error);
}

TEST(StrictInt, cast_throwing_to_wrapping) {
    isize a((1LL << 32) + 1); // does not fit in 32 bits
    auto wrap_a = int_cast<i32>(a);
    ASSERT_EQ(wrap_a, i32(1));
    isize b(-((1LL << 32) + 1));
    auto wrap_b = int_cast<i32>(b);
    ASSERT_EQ(wrap_b, i32(-1));
}

TEST(StrictInt, cast_narrow_throwing) {
    isize a(5000000000); // does not fit in 32 bits
    ASSERT_THROW(int_cast<throwing_i32>(a), std::overflow_error);
    isize b(-5000000000);
    ASSERT_THROW(int_cast<throwing_i32>(b), std::underflow_error);
}
