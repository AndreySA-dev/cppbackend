#include <catch2/catch_test_macros.hpp>
#include <iostream>

#include "../src/tv.h"

namespace Catch {

template <>
struct StringMaker<std::nullopt_t> {
    static std::string convert(std::nullopt_t) {
        using namespace std::literals;
        return "nullopt"s;
    }
};

template <typename T>
struct StringMaker<std::optional<T>> {
    static std::string convert(const std::optional<T>& opt_value) {
        if (opt_value) {
            return StringMaker<T>::convert(*opt_value);
        } else {
            return StringMaker<std::nullopt_t>::convert(std::nullopt);
        }
    }
};

}  // namespace Catch

SCENARIO("TV", "[TV]") {
    GIVEN("A TV") {  // Дано: Телевизор
        TV tv;

        // Изначально он выключен и не показывает никаких каналов
        SECTION("Initially it is off and doesn't show any channel") {
            CHECK(!tv.IsTurnedOn());
            CHECK(!tv.GetChannel().has_value());
        }

        // Когда он выключен,
        WHEN("it is turned off") {
            REQUIRE(!tv.IsTurnedOn());

// Включите эту секцию и доработайте класс TV, чтобы он проходил проверки в ней
#if 1
            // он не может переключать каналы
            THEN("it can't select any channel") {
                CHECK_THROWS_AS(tv.SelectChannel(10), std::logic_error);
                CHECK(tv.GetChannel() == std::nullopt);
                tv.TurnOn();
                CHECK(tv.GetChannel() == 1);
            }
#endif
        }

        WHEN("it is turned on first time") {  // Когда его включают в первый раз,
            tv.TurnOn();

            // то он включается и показывает канал #1
            THEN("it is turned on and shows channel #1") {
                CHECK(tv.IsTurnedOn());
                CHECK(tv.GetChannel() == 1);

                // А когда его выключают,
                AND_WHEN("it is turned off") {
                    tv.TurnOff();

                    // то он выключается и не показывает никаких каналов
                    THEN("it is turned off and doesn't show any channel") {
                        CHECK(!tv.IsTurnedOn());
                        CHECK(tv.GetChannel() == std::nullopt);
                    }
                }
            }
            // И затем может выбирать канал с 1 по 99
            AND_THEN("it can select channel from 1 to 99") {
                tv.SelectChannel(2);
				CHECK(tv.GetChannel() == 2);
                tv.SelectChannel(99);
				CHECK(tv.GetChannel() == 99);
                tv.SelectChannel(50);
				CHECK(tv.GetChannel() == 50);
            }
            /* Реализуйте самостоятельно остальные тесты */
        }
    }
}

TEST_CASE("TV second part", "[TV2]") {
	TV tv;
	tv.TurnOn();
	REQUIRE(tv.IsTurnedOn());
	
	SECTION("Select wrong channel") {

		CHECK(tv.GetChannel() == 1);
		CHECK_THROWS_AS(tv.SelectChannel(-5), std::out_of_range);
		CHECK_THROWS_AS(tv.SelectChannel(105), std::out_of_range);
	}

	SECTION("Test SelectLastViewedChannel") {
		CHECK(tv.GetChannel() == 1);
		tv.SelectChannel(5);
		CHECK(tv.GetChannel() == 5);
		tv.SelectLastViewedChannel();
		CHECK(tv.GetChannel() == 1);
		
		tv.SelectChannel(15);
		tv.SelectChannel(21);
		tv.SelectLastViewedChannel();
		CHECK(tv.GetChannel() == 15);
		tv.SelectLastViewedChannel();
		CHECK(tv.GetChannel() == 21);

		tv.TurnOff();
		CHECK_THROWS_AS(tv.SelectLastViewedChannel(), std::logic_error);

	}


}