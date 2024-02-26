#pragma once

#include "types/Types.h"
#include "item/Item.h"
#include "item/Products.h"

#include <nlohmann/json_fwd.hpp>

namespace Game3 {
	class Game;

	class Crop: public NamedRegisterable {
		public:
			Identifier customType;
			std::vector<Identifier> stages;
			Products products;
			double chance;
			bool canSpawnInTown;
			// TODO: refactor into subclasses with CropFactory
			nlohmann::json customData;

			Crop(Identifier, Identifier custom_type, std::vector<Identifier> stages_, Products products_, double chance_, bool can_spawn_in_town, nlohmann::json custom_data);
			Crop(Identifier, const std::shared_ptr<Game> &, const nlohmann::json &);

			const Identifier & getFirstStage() const;
			const Identifier & getLastStage() const;

		private:
			static Identifier getCustomType(const nlohmann::json &);
			static nlohmann::json getCustomData(const nlohmann::json &);
			static bool getCanSpawnInTown(const nlohmann::json &);
	};
}
