#include <gtest/gtest.h>

#include "../src/urlencode.h"

using namespace std::literals;

TEST(UrlEncodeTestSuite, OrdinaryCharsAreNotEncoded) {
    EXPECT_EQ(UrlEncode(""sv), ""s);
    EXPECT_EQ(UrlEncode("Hello World!"sv), "Hello%20World%21"s);
    EXPECT_EQ(UrlEncode("Hello"sv), "Hello"s);
    EXPECT_TRUE(UrlEncode("abc*"sv)  == "abc%2A"s || UrlEncode("abc*"sv)  == "abc%2a"s);
    EXPECT_EQ(UrlEncode("Hello World\v"sv), "Hello%20World%0B"s);
}

/* Напишите остальные тесты самостоятельно */
