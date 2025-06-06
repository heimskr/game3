#include "types/Layer.h"
#include "entity/ClientPlayer.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/RenderOptions.h"
#include "graphics/Tileset.h"
#include "item/Copier.h"
#include "packet/SetCopierConfigurationPacket.h"
#include "packet/UseItemPacket.h"
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
			const auto &object = stack.data.as_object();
			if (const auto *value = object.if_contains("positions")) {
				return boost::json::value_to<C<Position>>(*value);
			}
			return {};
		}
	}

	bool Copier::use(Slot, const ItemStackPtr &stack, const std::shared_ptr<Player> &player, Modifiers) {
		std::cout << getString(stack, player->getRealm()) << '\n';
		return true;
	}

	std::string Copier::getString(const ItemStackPtr &stack, const RealmPtr &realm) {
		// TODO: fluids

		const std::set<Position> positions = getPositions<std::set>(*stack);

		if (positions.empty()) {
			return {};
		}

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
			if (!last_position || *last_position + Position(0, 1) != *iter) {
				anchor = *iter;
			}
			last_position = *iter;
			++spans[*anchor];
		}

		std::map<Identifier, size_t> identifier_map{{"base:tile/empty", 0}};
		std::vector<std::string> identifiers{"empty"};

		for (const auto [anchor, span_length]: spans) {
			Position position = anchor;

			for (size_t i = 0; i < span_length; ++i) {
				for (const Layer layer: allLayers) {
					if (std::optional<TileID> tile = realm->tryTile(layer, position)) {
						Identifier identifier = tileset[*tile];
						if (!identifier_map.contains(identifier)) {
							identifier_map[identifier] = identifier_map.size();
							if (identifier.inSpace("base") && identifier.getPath() == "tile") {
								identifiers.emplace_back(identifier.getPostPath());
							} else {
								identifiers.emplace_back(identifier);
							}
						}
					}
				}

				++position.column;
			}
		}

		std::stringstream ss;
		ss << join(identifiers, ";") << '/';
		for (const auto [anchor, span_length]: spans) {
			ss << (anchor.row - min_row) << ',' << (anchor.column - min_column) << '=';
			Position position = anchor;
			bool first = true;

			std::string last_joined;

			for (size_t i = 0; i < span_length; ++i) {
				std::vector<size_t> layers;

				for (const Layer layer: allLayers) {
					std::optional<TileID> tile = realm->tryTile(layer, position);
					if (!tile) {
						break;
					}
					layers.push_back(identifier_map[tileset[*tile]]);
				}

				while (!layers.empty() && layers.back() == 0) {
					layers.pop_back();
				}

				if (first) {
					first = false;
				} else {
					ss << ',';
				}

				std::string joined = join(layers, ":");

				if (joined != last_joined) {
					ss << joined;
					last_joined = joined;
				}

				++position.column;
			}

			ss << '/';
		}

		std::string combined = std::move(ss).str();
		if (!combined.empty() && combined.back() == '/') {
			combined.pop_back();
		}

		if (const auto *value = stack->data.as_object().if_contains("includeTileEntities"); !value || !value->as_bool()) {
			return combined;
		}

		combined += '\n';
		ss = {};

		for (const Position &position: positions) {
			TileEntityPtr tile_entity = realm->tileEntityAt(position);
			if (!tile_entity) {
				continue;
			}
			boost::json::value json;
			tile_entity->toJSON(json);
			auto &object = json.as_object();
			object.erase("gid");
			object.erase("position");
			ss << (position.row - min_row) << ',' << (position.column - min_column) << '=' << boost::json::serialize(object) << '\n';
		}

		combined += ss.str();

		while (combined.back() == '\n') {
			combined.pop_back();
		}

		return combined;
	}

	bool Copier::drag(Slot, const ItemStackPtr &stack, const Place &place, Modifiers modifiers, std::pair<float, float>, DragAction) {
		if (modifiers == Modifiers(true, true, false, false)) {
			std::string tiles = getString(stack, place.realm);
			try {
				Paster(std::string_view(tiles)).paste(place.realm, place.position);
			} catch (const std::exception &err) {
				ERR("Couldn't paste: {}", err.what());
			}
			return true;
		}

		{
			auto lock = stack->data.uniqueLock();

			if (modifiers.onlyCtrl()) {
				if (auto *object = stack->data.if_object()) {
					object->erase("positions");
					object->erase("min");
				}
			} else if (auto *object = stack->data.if_object()) {
				const Position &position = place.position;
				std::unordered_set<Position> positions;

				if (auto *value = object->if_contains("positions")) {
					positions = boost::json::value_to<decltype(positions)>(*value);
				}

				if (auto iter = positions.find(position); iter != positions.end()) {
					positions.erase(iter);
					if (positions.empty()) {
						object->erase("min");
					} else if (auto *value = object->if_contains("min")) {
						auto &min = value->as_array();
						if (position.row == min[0] || position.column == min[1]) {
							std::optional<Position> minimums = computeMinimums(positions);
							assert(minimums);
							(*object)["min"] = boost::json::value_from(*minimums);
						}
					}
				} else {
					positions.insert(position);
					if (auto *value = object->if_contains("min")) {
						auto &min = value->as_array();
						min[0] = std::min(boost::json::value_to<Index>(min[0]), position.row);
						min[1] = std::min(boost::json::value_to<Index>(min[1]), position.column);
					} else {
						(*object)["min"] = boost::json::value_from(position);
					}
				}

				if (positions.empty()) {
					object->erase("min");
					object->erase("positions");
				} else {
					(*object)["positions"] = boost::json::value_from(positions);
				}
			}
		}

		place.player->getInventory(0)->notifyOwner({});
		return true;
	}

	void Copier::renderEffects(Window &, const RendererContext &context, const Position &mouse_position, Modifiers modifiers, const ItemStackPtr &stack) const {
		RectangleRenderer &rectangle = context.rectangle;

		std::unordered_set<Position> positions = getPositions(*stack);

		for (const Position &position: positions) {
			rectangle.drawOnMap(RenderOptions{
				.x = double(position.column),
				.y = double(position.row),
				.sizeX = 1.,
				.sizeY = 1.,
				.color = {1.f, 1.f, 0.f, .5f},
			});
		}

		if (modifiers == Modifiers(true, true, false, false)) {
			if (auto *value = stack->data.as_object().if_contains("min")) {
				Position anchor = boost::json::value_to<Position>(*value);

				for (const Position &position: positions) {
					Position adjusted = position - anchor + mouse_position;
					rectangle.drawOnMap(RenderOptions{
						.x = double(adjusted.column),
						.y = double(adjusted.row),
						.sizeX = 1.,
						.sizeY = 1.,
						.color = {.5f, .5f, .5f, .5f},
					});
				}
			}
		}
	}

	/*
	bool Copier::populateMenu(const InventoryPtr &inventory, Slot slot, const ItemStackPtr &stack, Glib::RefPtr<Gio::Menu> menu, Glib::RefPtr<Gio::SimpleActionGroup> group) const {
		std::weak_ptr<Player> weak_player(stack->getGame()->toClient().getPlayer());

		group->add_action("copier_copy", [slot, weak_player] {
			if (PlayerPtr player = weak_player.lock())
				player->send(make<UseItemPacket>(slot, Modifiers{}));
		});

		group->add_action("copier_include_tile_entities", [slot, weak_player] {
			if (PlayerPtr player = weak_player.lock())
				player->send(make<SetCopierConfigurationPacket>(slot, true));
		});

		group->add_action("copier_exclude_tile_entities", [slot, weak_player] {
			if (PlayerPtr player = weak_player.lock())
				player->send(make<SetCopierConfigurationPacket>(slot, false));
		});

		if (inventory == stack->getGame()->toClient().getPlayer()->getInventory(0)) {
			menu->append("Copy", "inventory_popup.copier_copy");

			if (auto iter = stack->data.find("includeTileEntities"); iter != stack->data.end() && iter->get<bool>())
				menu->append("Exclude Tile Entities", "inventory_popup.copier_exclude_tile_entities");
			else
				menu->append("Include Tile Entities", "inventory_popup.copier_include_tile_entities");
		}

		return true;
	}
	*/

	std::optional<Position> Copier::computeMinimums(const std::unordered_set<Position> &positions) {
		if (positions.empty())
			return std::nullopt;

		Position out = *positions.begin();

		for (const Position &position: positions) {
			out.row = std::min(out.row, position.row);
			out.column = std::min(out.column, position.column);
		}

		return out;
	}
}
