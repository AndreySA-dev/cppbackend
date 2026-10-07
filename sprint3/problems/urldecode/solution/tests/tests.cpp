#define BOOST_TEST_MODULE urlencode tests
#include <boost/test/unit_test.hpp>

#include "../src/urldecode.h"

BOOST_AUTO_TEST_CASE(UrlDecode_tests) {
    using namespace std::literals;

    BOOST_TEST(UrlDecode(""sv) == ""s);
    BOOST_TEST(UrlDecode("Hello%20World !"sv) == "Hello World !"s);
    BOOST_TEST(UrlDecode("Hello+World !"sv) == "Hello World !"s);
    BOOST_TEST(UrlDecode("%3BHello+World !"sv) == ";Hello World !"s);
    BOOST_TEST(UrlDecode("Hello%20World%3D!"sv) == "Hello World=!"s);
	try {
		UrlDecode("Hello%2TWorld!"sv);
		BOOST_TEST(false);

	} catch (...) {

	}

	try {
		UrlDecode("Hello%FWorld!"sv);
		BOOST_TEST(false);

	} catch (...) {

	}
    // Напишите остальные тесты для функции UrlDecode самостоятельно
}