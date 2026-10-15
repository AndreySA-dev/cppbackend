#include <cmath>
#include <catch2/catch_test_macros.hpp>

#include "../src/helper.h"

using namespace std::literals;
SCENARIO("Helper test") {
	INFO("Start helper tetst");

    GIVEN("Two double with are close values") {
		double d1 = 5.0000000001;
		double d2 = 5.00000000011;

        WHEN("try compare") {
            THEN("d2 greater d1") {
				CHECK(helper::LessOrEqual(d1, d2));
				CHECK(helper::GreatOrEqual(d2, d1));
            }
        }

    }

}
