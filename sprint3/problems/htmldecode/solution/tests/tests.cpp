#include <catch2/catch_test_macros.hpp>

#include "../src/htmldecode.h"

using namespace std::literals;

TEST_CASE("Text without mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode(""sv) == ""s);
    CHECK(HtmlDecode("hello"sv) == "hello"s);
    CHECK(HtmlDecode("Johnson&ampJohnson"sv) == "Johnson&Johnson"s);
    CHECK(HtmlDecode("Johnson&amp;Johnson"sv) == "Johnson&Johnson"s);
    CHECK(HtmlDecode("Johnson&AMPJohnson"sv) == "Johnson&Johnson"s);
    CHECK(HtmlDecode("Johnson&AMP;Johnson"sv) == "Johnson&Johnson"s);
    CHECK(HtmlDecode("Johnson&Johnson"sv) == "Johnson&Johnson"s);
    CHECK(HtmlDecode("Johnson&XYZJohnson"sv) == "Johnson&XYZJohnson"s);
    CHECK(HtmlDecode("Johnson&XYZ;Johnson"sv) == "Johnson&XYZ;Johnson"s);
    CHECK(HtmlDecode("Can&apost"sv) == "Can't"s);
    CHECK(HtmlDecode("Can&aPost"sv) == "Can&aPost"s);
    CHECK(HtmlDecode("&quotCPP&quot"sv) == "\"CPP\""s);
    CHECK(HtmlDecode("&quotCPP&quot;"sv) == "\"CPP\""s);
    CHECK(HtmlDecode("10 &lt 100"sv) == "10 < 100"s);
    CHECK(HtmlDecode("50 &GT 49"sv) == "50 > 49"s);
}

// Напишите недостающие тесты самостоятельно
