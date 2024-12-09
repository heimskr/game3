#include "game/Crop.h"
#include "game/Game.h"

#include <cassert>

namespace Game3 {
	Crop::Crop(Identifier identifier_, Identifier custom_type, std::vector<Identifier> stages_, Products products_, double chance_, bool can_spawn_in_town, boost::json::value custom_data):
		NamedRegisterable(std::move(identifier_)),
		customType(std::move(custom_type)),
		stages(std::move(stages_)),
		products(std::move(products_)),
		chance(chance_),
		canSpawnInTown(can_spawn_in_town),
		customData(std::move(custom_data)) {
			assert(!stages.empty());
		}

	Crop::Crop(Identifier identifier_, const GamePtr &game, const boost::json::value &json):
		Crop(
			std::move(identifier_),
			getCustomType(json),
			boost::json::value_to<std::vector<Identifier>>(json.at("stages")),
			boost::json::value_to<Products>(json.at("products"), game),
			json.at("chance").as_double(),
			getCanSpawnInTown(json),
			getCustomData(json)
		) {}

	const Identifier & Crop::getFirstStage() const {
		if (stages.empty())
			throw std::runtime_error("No crop stages found");
		return stages.front();
	}

	const Identifier & Crop::getLastStage() const {
		if (stages.empty())
			throw std::runtime_error("No crop stages found");
		return stages.back();
	}

	Identifier Crop::getCustomType(const boost::json::value &json) {
		const auto &object = json.as_object();
		if (auto iter = object.find("type"); iter != object.end()) {
			return boost::json::value_to<Identifier>(iter->value());
		}
		return {};
	}

	boost::json::value Crop::getCustomData(const boost::json::value &json) {
		const auto &object = json.as_object();
		if (auto iter = object.find("custom"); iter != object.end()) {
			return iter->value();
		}
		return {};
	}

	bool Crop::getCanSpawnInTown(const boost::json::value &json) {
		const auto &object = json.as_object();
		if (auto iter = object.find("canSpawnInTown"); iter != object.end()) {
			return iter->value().as_bool();
		}
		return {};
	}
}
