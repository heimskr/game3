#pragma once

#include "Layer.h"
#include "data/Identifier.h"

#include <map>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace Game3 {
	class Realm;
	struct Position;

	class Paster {
		public:
			Paster() = default;
			Paster(std::string_view);

			void ingest(std::string_view);
			void paste(const std::shared_ptr<Realm> &, const Position &anchor);

		private:
			std::vector<Identifier> identifiers;
			std::map<Position, std::array<Identifier *, LAYER_COUNT>> tiles;
			std::map<Position, nlohmann::json> tileEntityJSON;
	};
}
