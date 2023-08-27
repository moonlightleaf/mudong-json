#include <gtest/gtest.h>

#include <iostream>

#include <Value.hpp>

using namespace mudong;

inline void TEST_BOOL(bool b) {
    json::Value V(b);
    EXPECT_EQ(b, V.getBool());
    EXPECT_EQ(json::ValueType::TYPE_BOOL, V.getType());
}

inline void TEST_NULL() {
    json::Value V;
    EXPECT_EQ(json::ValueType::TYPE_NULL, V.getType());
}

inline void TEST_DOUBLE(double d) {
    json::Value V(d);
    EXPECT_EQ(d, V.getDouble());
    EXPECT_EQ(json::ValueType::TYPE_DOUBLE, V.getType());
}

inline void TEST_INT32(int32_t i32) {
    json::Value V(i32);
    EXPECT_EQ(i32, V.getInt32());
    EXPECT_EQ(json::ValueType::TYPE_INT32, V.getType());
}

inline void TEST_INT64(int64_t i64) {
    json::Value V(i64);
    EXPECT_EQ(i64, V.getInt64());
    EXPECT_EQ(json::ValueType::TYPE_INT64, V.getType());
}

inline void TEST_STRING(const std::string_view str) {
    json::Value V(str);
    EXPECT_EQ(str, V.getStringView());
    EXPECT_EQ(json::ValueType::TYPE_STRING, V.getType());
}

TEST(json_value, null) {
    TEST_NULL();
}

TEST(json_value, bool_) {
    TEST_BOOL(true);
    TEST_BOOL(false);
}

TEST(json_value, double_) {
    TEST_DOUBLE(0.0);
    TEST_DOUBLE(876.543e21);
    TEST_DOUBLE(-111.000e-010);
    TEST_DOUBLE(2.2250738585072014e-308);
    TEST_DOUBLE(-2.2250738585072014e-308);
    TEST_DOUBLE(1.7976931348623157e308);
    TEST_DOUBLE(-1.7976931348623157e308);
}

TEST(json_value, int32_) {
    TEST_INT32(0);
    TEST_INT32(-0);
    TEST_INT32(1);
    TEST_INT32(-1);
    TEST_INT32(2147483647);
    TEST_INT32(-2147483648);
}

TEST(json_value, int64_) {
    TEST_INT64(2147483648LL);
    TEST_INT64(-2147483649LL);
    TEST_INT64(std::numeric_limits<int64_t>::max());
    TEST_INT64(std::numeric_limits<int64_t>::min());

    TEST_INT64(0);
    TEST_INT64(-1);
    TEST_INT64(2147483647);
    TEST_INT64(-2147483648);
}

TEST(json_value, string_) {
    TEST_STRING("");
    TEST_STRING("abcd");
    TEST_STRING("\n");
    TEST_STRING("\\n");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

