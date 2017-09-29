#include <gtest/gtest.h>

#include <strict_int/strict_int.hpp>

using namespace strict_int;

TEST(StrictInt, Main) {
    ASSERT_EQ(strict_int::foo(), 123);
}
