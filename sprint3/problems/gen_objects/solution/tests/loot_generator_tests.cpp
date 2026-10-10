#include "../src/json_loader.h"
#include "../src/loot_generator.h"
#include "../src/model.h"

#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <sstream>
#include <string>


using namespace std;
using namespace std::literals;

		string json_cfg{R"(
{
  "defaultDogSpeed": 3.0,
  "lootGeneratorConfig": {
    "period": 5.0,
    "probability": 0.5
  },
  "maps": [
    {
      "dogSpeed": 4.0,
      "id": "map1",
      "name": "Map 1",
      "lootTypes": [
        {
          "name": "key",
          "file": "assets/key.obj",
          "type": "obj",
          "rotation": 90,
          "color" : "#338844",
          "scale": 0.03
        },
        {
          "name": "wallet",
          "file": "assets/wallet.obj",
          "type": "obj",
          "rotation": 0,
          "color" : "#883344",
          "scale": 0.01
        }
      ],
      "roads": [
        {
          "x0": 0,
          "y0": 0,
          "x1": 40
        },
        {
          "x0": 40,
          "y0": 0,
          "y1": 30
        },
        {
          "x0": 40,
          "y0": 30,
          "x1": 0
        },
        {
          "x0": 0,
          "y0": 0,
          "y1": 30
        }
      ],
      "buildings": [
        {
          "x": 5,
          "y": 5,
          "w": 30,
          "h": 20
        }
      ],
      "offices": [
        {
          "id": "o0",
          "x": 40,
          "y": 30,
          "offsetX": 5,
          "offsetY": 0
        }
      ]
    },
    {
      "id": "town",
      "name": "Town",
      "lootTypes": [
        {
          "name": "key",
          "file": "assets/key.obj",
          "type": "obj",
          "scale": 0.03
        },
        {
          "name": "wallet",
          "file": "assets/wallet.obj",
          "type": "obj",
          "scale": 0.01
        }
      ],
      "roads": [
        {
          "x0": 0,
          "y0": 0,
          "x1": 40
        },
        {
          "x0": 40,
          "y0": 0,
          "y1": 30
        },
        {
          "x0": 40,
          "y0": 30,
          "x1": 0
        },
        {
          "x0": 0,
          "y0": 15,
          "x1": 40
        },
        {
          "x0": 20,
          "y0": 0,
          "y1": 30
        },
        {
          "x0": 0,
          "y0": 22,
          "x1": 17
        },
        {
          "x0": 17,
          "y0": 18,
          "y1": 27
        },
        {
          "x0": 10,
          "y0": 22,
          "y1": 30
        },
        {
          "x0": 0,
          "y0": 10,
          "x1": 10
        },
        {
          "x0": 10,
          "y0": 10,
          "y1": 5
        },
        {
          "x0": 10,
          "y0": 5,
          "x1": 20
        },
        {
          "x0": 20,
          "y0": 30,
          "y1": 40
        },
        {
          "x0": 20,
          "y0": 40,
          "x1": 10
        },
        {
          "x0": 20,
          "y0": 40,
          "x1": 30
        },
        {
          "x0": 30,
          "y0": 0,
          "y1": 10
        },
        {
          "x0": 20,
          "y0": 25,
          "x1": 25
        },
        {
          "x0": 30,
          "y0": 25,
          "y1": 30
        },
        {
          "x0": 20,
          "y0": 20,
          "x1": 25
        },
        {
          "x0": 30,
          "y0": 15,
          "y1": 20
        },
        {
          "x0": 35,
          "y0": 20,
          "x1": 40
        },
        {
          "x0": 40,
          "y0": 25,
          "x1": 35
        },
        {
          "x0": 30,
          "y0": 30,
          "y1": 35
        }
      ],
      "buildings": [
        {
          "x": 2,
          "y": 2,
          "w": 6,
          "h": 6
        },
        {
          "x": 12,
          "y": 7,
          "w": 6,
          "h": 6
        },
        {
          "x": 22,
          "y": 2,
          "w": 6,
          "h": 11
        },
        {
          "x": 32,
          "y": 2,
          "w": 6,
          "h": 11
        },
        {
          "x": 22,
          "y": 16,
          "w": 4,
          "h": 3
        },
        {
          "x": 33,
          "y": 16,
          "w": 4,
          "h": 3
        },
        {
          "x": 34,
          "y": 21,
          "w": 4,
          "h": 3
        },
        {
          "x": 22,
          "y": 21,
          "w": 4,
          "h": 3
        },
        {
          "x": 22,
          "y": 26,
          "w": 5,
          "h": 3
        },
        {
          "x": 34,
          "y": 26,
          "w": 5,
          "h": 3
        },
        {
          "x": 28,
          "y": 21,
          "w": 4,
          "h": 3
        },
        {
          "x": 2,
          "y": 16,
          "w": 5,
          "h": 5
        },
        {
          "x": 9,
          "y": 16,
          "w": 6,
          "h": 3
        },
        {
          "x": 12,
          "y": 24,
          "w": 3,
          "h": 4
        },
        {
          "x": 2,
          "y": 24,
          "w": 6,
          "h": 4
        },
        {
          "x": 12,
          "y": 1,
          "w": 7,
          "h": 2
        },
        {
          "x": 11,
          "y": 34,
          "w": 4,
          "h": 4
        },
        {
          "x": 22,
          "y": 31,
          "w": 6,
          "h": 2
        },
        {
          "x": 22,
          "y": 35,
          "w": 6,
          "h": 4
        },
        {
          "x": 17,
          "y": 41,
          "w": 7,
          "h": 4
        }
      ],
      "offices": [
        {
          "id": "o0",
          "x": 40,
          "y": 30,
          "offsetX": 5,
          "offsetY": 0
        }
      ]
    }
  ]
}
		)"};

SCENARIO("Loot generation") {
	using loot_gen::LootGenerator;
	using TimeInterval = LootGenerator::TimeInterval;

	GIVEN("a loot generator") {
		LootGenerator gen{1s, 1.0};

		constexpr TimeInterval TIME_INTERVAL = 1s;

		WHEN("loot count is enough for every looter") {
			THEN("no loot is generated") {
				for (unsigned looters = 0; looters < 10; ++looters) {
					for (unsigned loot = looters; loot < looters + 10; ++loot) {
						INFO("loot count: " << loot << ", looters: " << looters);
						REQUIRE(gen.Generate(TIME_INTERVAL, loot, looters) == 0);
					}
				}
			}
		}

		WHEN("number of looters exceeds loot count") {
			THEN("number of loot is proportional to loot difference") {
				for (unsigned loot = 0; loot < 10; ++loot) {
					for (unsigned looters = loot; looters < loot + 10; ++looters) {
						INFO("loot count: " << loot << ", looters: " << looters);
						REQUIRE(gen.Generate(TIME_INTERVAL, loot, looters) == looters - loot);
					}
				}
			}
		}
	}

	GIVEN("a loot generator with some probability") {
		constexpr TimeInterval BASE_INTERVAL = 1s;
		LootGenerator gen{BASE_INTERVAL, 0.5};

		WHEN("time is greater than base interval") {
			THEN("number of generated loot is increased") {
				CHECK(gen.Generate(BASE_INTERVAL * 2, 0, 4) == 3);
			}
		}

		WHEN("time is less than base interval") {
			THEN("number of generated loot is decreased") {
				const auto time_interval = std::chrono::duration_cast<TimeInterval>(
					std::chrono::duration<double>{1.0 / (std::log(1 - 0.5) / std::log(1.0 - 0.25))});
				CHECK(gen.Generate(time_interval, 0, 4) == 1);
			}
		}
	}

	GIVEN("a loot generator with custom random generator") {
		LootGenerator gen{1s, 0.5, [] { return 0.5; }};
		WHEN("loot is generated") {
			THEN("number of loot is proportional to random generated values") {
				const auto time_interval = std::chrono::duration_cast<TimeInterval>(
					std::chrono::duration<double>{1.0 / (std::log(1 - 0.5) / std::log(1.0 - 0.25))});
				CHECK(gen.Generate(time_interval, 0, 4) == 0);
				CHECK(gen.Generate(time_interval, 0, 4) == 1);
			}
		}
	}
}

SCENARIO("Game config load and check LootType parameters") {
	GIVEN("JSON text") {

		WHEN("load config") {
			std::stringstream json_stream(json_cfg);
			THEN("No exception, and load expected parameters") {
				model::Game game;
				REQUIRE_NOTHROW(game = json_loader::LoadGame(json_stream));
				REQUIRE(game.GetMaps().size() == 2);
				CHECK(game.GetLootSpawnPeriod() == 5.000);
				CHECK(game.GetLootSpawnProbability() == 0.5);
				CHECK((game.FindMap(model::Map::Id{"town"})->GetLootTypes().size()) == 2 );
				CHECK((game.FindMap(model::Map::Id{"town"})->GetLootTypes().size()) == 2 );
			}
		}
	}
}
