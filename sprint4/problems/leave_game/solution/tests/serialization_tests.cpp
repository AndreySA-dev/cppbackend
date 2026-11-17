#define _USE_MATH_DEFINES

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <catch2/catch_test_macros.hpp>

#include "../src/authenticator.h"
#include "../src/game_handler.h"
#include "../src/game_serialization.h"
#include "../src/json_loader.h"
#include "../src/loot_generator.h"
#include "../src/model.h"

#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#include <boost/asio.hpp>

using namespace std::literals;
using namespace std;

using InputArchive = boost::archive::text_iarchive;
using OutputArchive = boost::archive::text_oarchive;

namespace net = boost::asio;

struct Fixture {
	std::ostringstream strm{ios::binary};
	OutputArchive output_archive{strm};
	string data;
};

SCENARIO_METHOD(Fixture, "game state serialization") {

	GIVEN("A point") {
		const model::Point p{123, 456};
		WHEN("point is serialized") {
			output_archive << p;


			THEN("it is equal to point after serialization") {
				data = strm.str();
				istringstream iss(data);
				InputArchive input_archive{iss};
				model::Point restored_point;
				input_archive >> restored_point;
				CHECK(p.x == restored_point.x);
				CHECK(p.y == restored_point.y);
			}
		}
	}

	GIVEN("A map with loot and dogs") {

		const auto map_jv = json_loader::ParseFile("./tests/config_test_map.json");
		model::Map init_map = json_loader::JSONToMap(map_jv.as_object());

		init_map.AddLoot(init_map.GetLootTypes()[0].id, {5, 9});
		init_map.AddLoot(init_map.GetLootTypes()[1].id, {15, 21});

		init_map.AddDog(model::Dog::Id(11));
		init_map.AddDog(model::Dog::Id(22));

		model::Loot dog_loot;
		dog_loot.id = model::Loot::Id{33};
		dog_loot.loot_type = &init_map.GetLootTypes()[1];
		init_map.GetDog(model::Dog::Id(22))->AddLoot(dog_loot);

		dog_loot.id = model::Loot::Id{44};
		dog_loot.loot_type = &init_map.GetLootTypes()[0];
		init_map.GetDog(model::Dog::Id(22))->AddLoot(dog_loot);

		WHEN("loot is serialized") {
			serialization::LootRepr loot_repr(init_map.GetLoots()[1]);
			output_archive << loot_repr;
			THEN("after deserialized loot has initial value") {
				data = strm.str();
				istringstream iss(data);
				InputArchive input_archive{iss};
				serialization::LootRepr lr;
				input_archive >> lr;
				model::Loot loot = lr.Restore(init_map);
				CHECK(loot.loot_type->id == init_map.GetLootTypes()[1].id);
				CHECK(loot.loot_type->color == init_map.GetLootTypes()[1].color);
				CHECK(loot.loot_type->name == init_map.GetLootTypes()[1].name);
				CHECK(loot.position.x == 15);
				CHECK(loot.position.y == 21);
			}
		}
		AND_WHEN("dog is serialized") {
			serialization::DogRepr dog_repr(*init_map.GetDog(model::Dog::Id{22}));
			output_archive << dog_repr;
			THEN("after deserialized dog has initial value") {
				const auto map_jv = json_loader::ParseFile("./tests/config_test_map.json");
				model::Map restored_map = json_loader::JSONToMap(map_jv.as_object());

				data = strm.str();
				istringstream iss(data);
				InputArchive input_archive{iss};

				serialization::DogRepr dr;
				input_archive >> dr;
				const model::Dog& restored_dog = dr.Restore(restored_map);

				model::Dog init_dog1 = *init_map.GetDog(model::Dog::Id{22});
				CHECK(restored_dog.GetBagCapacity() == init_dog1.GetBagCapacity());
				CHECK(restored_dog.GetDirection() == init_dog1.GetDirection());
				CHECK(restored_dog.GetId() == init_dog1.GetId());
				CHECK(restored_dog.GetPosition().x == init_dog1.GetPosition().x);
				CHECK(restored_dog.GetPosition().y == init_dog1.GetPosition().y);
				CHECK(restored_dog.GetScore() == init_dog1.GetScore());
				CHECK(restored_dog.GetSpeed() == init_dog1.GetSpeed());
				CHECK(restored_dog.GetBag()[1].id == init_dog1.GetBag()[1].id);
				CHECK(restored_dog.GetBag()[1].loot_type->color == init_dog1.GetBag()[1].loot_type->color);
				CHECK(restored_dog.GetBag()[1].loot_type->name == init_dog1.GetBag()[1].loot_type->name);
			}
		}
		AND_WHEN("Map serializated") {
			serialization::MapRepr map_repr(init_map);
			output_archive << map_repr;
			THEN("after deserialization map has initial value") {
				data = strm.str();
				istringstream iss(data);
				InputArchive input_archive{iss};

				serialization::MapRepr mr;
				input_archive >> mr;
				model::Map restored_map = json_loader::JSONToMap(map_jv.as_object());
				mr.Restore(restored_map);
				CHECK(restored_map.GetLoots()[1].position.x == init_map.GetLoots()[1].position.x);
				CHECK(restored_map.GetLoots()[1].position.y == init_map.GetLoots()[1].position.y);
				CHECK(restored_map.GetLoots()[1].loot_type->name == init_map.GetLoots()[1].loot_type->name);

				CHECK((*restored_map.GetDog(model::Dog::Id{22})).GetBag()[1].loot_type->name ==
					  (*init_map.GetDog(model::Dog::Id{22})).GetBag()[1].loot_type->name);
				CHECK(restored_map.GetCurrentNextId() == init_map.GetCurrentNextId());
			}
		}
	}

	GIVEN("Full game state") {
		auto game = json_loader::LoadGame("./tests/config_test.json", "file#config_test_map.json");
		auth::Authenticator au(game);

		game.GetMaps()[0].AddLoot(model::LootType::Id(0), {10, 5});
		au.AddUser("Sharik", model::Map::Id("map1")); // <-- user ID = 0
		game.GetMaps()[0].AddDog(model::Dog::Id(0));
		auto dog = game.GetMaps()[0].GetDog(model::Dog::Id(0));
		dog->AddLoot(model::Loot{model::Loot::Id{11}, &game.GetMaps()[0].GetLootTypes()[0], {0, 0}});


		WHEN("game is serialized") {
			serialization::GameRepr game_repr(game);
			serialization::AuthRepr auth_repr(au);
			output_archive << game_repr;
			output_archive << auth_repr;
			THEN("Deserialize without exception") {
				data = strm.str();
				istringstream iss(data);
				InputArchive input_archive{iss};

				serialization::GameRepr gr;
				serialization::AuthRepr ar;
				input_archive >> gr;
				input_archive >> ar;
			}
		}
	}
}
