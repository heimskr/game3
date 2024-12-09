#pragma once

#include "types/Layer.h"
#include "data/Identifier.h"

#include <functional>
#include <map>
#include <string>
#include <vector>

#include <boost/json.hpp>

namespace Game3 {
	class Realm;
	struct Position;

	class Paster {
		public:
			Paster() = default;
			Paster(std::string_view);

			void ingest(std::string_view);
			/** Applies a patch to all the tile entity JSONs. */
			void patch(const boost::json::value &);
			void paste(const std::shared_ptr<Realm> &, const Position &anchor, bool destructive = true, const std::function<void()> &pre_send = {});

		private:
			std::vector<Identifier> identifiers;
			std::map<Position, std::array<Identifier *, LAYER_COUNT>> tiles;
			std::map<Position, boost::json::value> tileEntityJSON;
	};
}
