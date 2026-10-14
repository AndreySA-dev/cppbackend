#define _USE_MATH_DEFINES

#include "../src/collision_detector.h"

#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>


namespace collision_detector {

using Items = std::vector<Item>;
using Gatherers = std::vector<Gatherer>;
using Events = std::vector<GatheringEvent>;


class ItemGathererImpl : public ItemGathererProvider {

  public:
	ItemGathererImpl(const Items& items, const Gatherers& gatherers) : items_(items), gatherers_(gatherers) {}
	size_t ItemsCount() const override {
		return items_.size();
	}

	Item GetItem(size_t idx) const override {
		return items_[idx];
	}

	size_t GatherersCount() const override {
		return gatherers_.size();
	}

	Gatherer GetGatherer(size_t idx) const override {
		return gatherers_[idx];
	}

  private:
	const Items items_;
	const Gatherers gatherers_;
};

bool IsEq(double a, double b, double eps = 1E-9) {
	return std::fabs(a - b) <= eps;
}

bool operator==(collision_detector::GatheringEvent left, collision_detector::GatheringEvent right) {
	return left.gatherer_id == right.gatherer_id && left.item_id == right.item_id &&
		   IsEq(left.sq_distance, right.sq_distance) && IsEq(left.time, right.time);
}

} // namespace collision_detector

// Напишите здесь тесты для функции collision_detector::FindGatherEvents

// clang-format off
const collision_detector::Items FIXTURE_ITEMS = {
	{{6.5, 10.5}, 0}, // x0
	{{8.5, 7.5}, 0}, // x1
	{{10, 7.5}, 0}, 
	{{4.5, 3}, 0},
	{{2.5, 3.5}, 0},
	{{8.5, 6}, 0}, // x5
	{{7, 6}, 0},
	{{6.5, 6.5}, 0},
	{{6.5, 8}, 0},
	{{5.5, 8.5}, 0},
	{{5, 7.5}, 0}, // x10
	{{9.5, 9.5}, 0},
	{{13.5, 13}, 0},
	{{2, 12}, 0},
	{{17, 2}, 0},
	{{1.5, 2}, 0},
	{{8, 1.5}, 0},
	{{4.5, 13.25}, 1}, // <----- item radius 1
	
};

const collision_detector::Gatherers FIXTURE_GATHERERS = {
	{{8, 2}, {8, 2}, 1},
	{{2.5, 2}, {10, 8}, 1},
	{{11, 5}, {4, 10}, 1},
	{{4.5, 7.5}, {14.5, 13}, 1},
	{{0, 15}, {10, 15}, 1},
};

const collision_detector::Events EXPECTED_EVENTS = {
	{10, 3, 0.0580614203454894, 0.0383877159309021},
	{9, 3, 0.155470249520154, 0.119001919385797},
	{8, 3, 0.276391554702494, 0.174664107485605},
	{3, 1, 0.219512195121952, 0.227642276422764},
	{5, 2, 0.408783783783783, 0.304054054054054},
	{1, 2, 0.33783783783784, 0.405405405405405},
	{17, 4, 3.0625, 0.45},
	{11, 3, 0.431861804222638, 0.468330134357006},
	{6, 1, 0.0975609756097526, 0.626016260162602},
	{8, 2, 0.030405405405407, 0.628378378378378},
	{5, 1, 0.390243902439011, 0.747967479674797},
	{9, 2, 0.121621621621628, 0.756756756756757},
	{1, 1, 0.298780487804862, 0.845528455284553},
	{12, 3, 0.232245681381954, 0.923224568138196},
	{2, 1, 0.152439024390219, 0.967479674796748}

};

const collision_detector::Items NOT_GATHERERS_ITEMS = {

};
// clang-format on


namespace Catch {
template <>
struct StringMaker<collision_detector::GatheringEvent> {
	static std::string convert(collision_detector::GatheringEvent const& value) {
		std::ostringstream tmp;
		tmp << std::setprecision(10) << "(" << value.gatherer_id << "," << value.item_id << "," << value.sq_distance
			<< "," << value.time << ")";

		return tmp.str();
	}
};
} // namespace Catch

SCENARIO("MOVE some dogs and catch some items") {
	GIVEN("a item gatherer") {
		using namespace collision_detector;
		ItemGathererImpl ig(FIXTURE_ITEMS, FIXTURE_GATHERERS);
		WHEN("find events with out exception") {
			std::vector<collision_detector::GatheringEvent> events;

			REQUIRE_NOTHROW(events = FindGatherEvents(ig));
			THEN(" give expected event values") {}
			AND_THEN("Event count equal expected count") {
				CHECK(events.size() == EXPECTED_EVENTS.size());
			}
			AND_THEN("All event is equal expected") {

				for (size_t i = 0; i < events.size(); ++i) {
					INFO("Check event number - " + std::to_string(i));
					CHECK(events[i] == EXPECTED_EVENTS[i]);
				}
			}
		}
	}
}