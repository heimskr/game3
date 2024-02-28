#include "Layer.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/RenderOptions.h"
#include "graphics/Tileset.h"
#include "item/Copier.h"
#include "realm/Realm.h"
#include "types/Position.h"
#include "tools/Paster.h"
#include "util/Util.h"

#include <sstream>

namespace Game3 {
	namespace {
		template <template <typename...> typename C = std::unordered_set>
		C<Position> getPositions(const ItemStack &stack) {
			auto lock = stack.data.sharedLock();
			if (auto iter = stack.data.find("positions"); iter != stack.data.end())
				return *iter;
			return {};
		}
	}

	bool Copier::use(Slot, ItemStack &stack, const std::shared_ptr<Player> &player, Modifiers) {
		std::cout << getTiles(stack, player->getRealm()) << '\n';
		return true;
	}

	std::string Copier::getTiles(const ItemStack &stack, const RealmPtr &realm) const {
		// TODO: fluids

		std::set<Position> positions = getPositions<std::set>(stack);

		if (positions.empty())
			return {};

		Tileset &tileset = realm->getTileset();
		std::map<Position, size_t> spans;
		std::optional<Position> last_position;
		std::optional<Position> anchor;

		const Index min_row = std::min_element(positions.begin(), positions.end(), [](const Position &one, const Position &two) {
			return one.row < two.row;
		})->row;

		const Index min_column = std::min_element(positions.begin(), positions.end(), [](const Position &one, const Position &two) {
			return one.column < two.column;
		})->column;

		Position top_left{min_row, min_column};

		for (auto iter = positions.begin(); iter != positions.end(); ++iter) {
			if (!last_position || *last_position + Position(0, 1) != *iter)
				anchor = *iter;
			last_position = *iter;
			++spans[*anchor];
		}

		std::map<Identifier, size_t> identifier_map{{"base:tile/empty", 0}};
		std::vector<std::string> identifiers{"empty"};

		for (const auto [anchor, span_length]: spans) {
			Position position = anchor;

			for (size_t i = 0; i < span_length; ++i) {
				for (const Layer layer: allLayers) {
					std::optional<TileID> tile = realm->tryTile(layer, position);
					if (!tile)
						continue;
					Identifier identifier = tileset[*tile];
					if (!identifier_map.contains(identifier)) {
						identifier_map[identifier] = identifier_map.size();
						if (identifier.inSpace("base") && identifier.getPath() == "tile")
							identifiers.push_back(identifier.getPostPath());
						else
							identifiers.push_back(identifier.str());
					}
				}

				++position.column;
			}
		}

		std::stringstream ss;
		ss << join(identifiers, ";") << '\n';
		for (const auto [anchor, span_length]: spans) {
			ss << (anchor.row - min_row) << ',' << (anchor.column - min_column) << '=';
			Position position = anchor;
			bool first = true;

			std::string last_joined;

			for (size_t i = 0; i < span_length; ++i) {
				std::vector<size_t> layers;

				for (const Layer layer: allLayers) {
					std::optional<TileID> tile = realm->tryTile(layer, position);
					if (!tile)
						break;
					layers.push_back(identifier_map[tileset[*tile]]);
				}

				while (!layers.empty() && layers.back() == 0)
					layers.pop_back();

				if (first)
					first = false;
				else
					ss << ',';

				std::string joined = join(layers, ":");

				if (joined != last_joined) {
					ss << joined;
					last_joined = joined;
				}

				++position.column;
			}

			ss << '\n';
		}

		std::string combined = ss.str();
		if (!combined.empty() && combined.back() == '\n')
			combined.pop_back();

		return combined;
	}

	bool Copier::drag(Slot, ItemStack &stack, const Place &place, Modifiers modifiers) {
		if (modifiers == Modifiers(true, true, false, false)) {
			std::string tiles = getTiles(stack, place.realm);
			Paster(std::string_view(tiles)).paste(*place.realm, place.position);
			return true;
		}

		{
			auto lock = stack.data.uniqueLock();

			if (modifiers.onlyCtrl()) {
				stack.data.erase("positions");
			} else {
				std::unordered_set<Position> positions;

				if (auto iter = stack.data.find("positions"); iter != stack.data.end())
					positions = *iter;

				if (auto iter = positions.find(place.position); iter != positions.end()) {
					positions.erase(iter);
				} else {
					positions.insert(place.position);
				}

				stack.data["positions"] = std::move(positions);
			}
		}

		place.player->getInventory(0)->notifyOwner();
		return true;
	}

	void Copier::renderEffects(const RendererContext &context, ItemStack &stack) const {
		RectangleRenderer &rectangle = context.rectangle;

		std::unordered_set<Position> positions = getPositions(stack);

		for (const Position &position: positions) {
			rectangle.drawOnMap(RenderOptions{
				.x = double(position.column),
				.y = double(position.row),
				.sizeX = 1.,
				.sizeY = 1.,
				.color = {1.f, 1.f, 0.f, .5f},
			});
		}
	}
}
