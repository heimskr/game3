#pragma once

#include "types/Types.h"
#include "item/Item.h"
#include "item/Products.h"

#include <boost/json/fwd.hpp>

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
			boost::json::value customData;

			Crop(Identifier, Identifier custom_type, std::vector<Identifier> stages_, Products products_, double chance_, bool can_spawn_in_town, boost::json::value custom_data);
			Crop(Identifier, const std::shared_ptr<Game> &, const boost::json::value &);

			const Identifier & getFirstStage() const;
			const Identifier & getLastStage() const;

		private:
			static Identifier getCustomType(const boost::json::value &);
			static boost::json::value getCustomData(const boost::json::value &);
			static bool getCanSpawnInTown(const boost::json::value &);
	};
}
