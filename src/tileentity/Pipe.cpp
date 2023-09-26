#include "Log.h"
#include "graphics/Tileset.h"
#include "game/EnergyContainer.h"
#include "game/Game.h"
#include "graphics/SpriteRenderer.h"
#include "pipes/EnergyNetwork.h"
#include "pipes/ItemNetwork.h"
#include "pipes/PipeNetwork.h"
#include "realm/Realm.h"
#include "tileentity/Pipe.h"
#include "util/Util.h"

#include <deque>

namespace Game3 {
	Pipe::Pipe(Position position_):
		TileEntity("base:tile/missing"_id, ID(), position_, false) {}

	Identifier Pipe::Corner(PipeType type) {
		switch (type) {
			case PipeType::Item:   return ItemCorner();
			case PipeType::Fluid:  return FluidCorner();
			case PipeType::Energy: return EnergyCorner();
			default: return {};
		}
	}

	Identifier Pipe::ExtractorsCorner(PipeType type) {
		switch (type) {
			case PipeType::Item:   return ItemExtractorsCorner();
			case PipeType::Fluid:  return FluidExtractorsCorner();
			case PipeType::Energy: return EnergyExtractorsCorner();
			default: return {};
		}
	}

	DirectionalContainer<std::shared_ptr<Pipe>> Pipe::getConnected(PipeType pipe_type) const {
		DirectionalContainer<std::shared_ptr<Pipe>> out;
		auto realm = getRealm();

		directions[pipe_type].iterate([&](Direction direction) {
			if (auto neighbor = realm->tileEntityAt(position + direction))
				if (auto neighbor_pipe = std::dynamic_pointer_cast<Pipe>(neighbor))
					if (neighbor_pipe->directions[pipe_type].has(flipDirection(direction)))
						out[direction] = neighbor_pipe;
		});

		return out;
	}

	std::shared_ptr<Pipe> Pipe::getConnected(PipeType pipe_type, Direction direction) const {
		auto realm = getRealm();

		if (auto neighbor = realm->tileEntityAt(position + direction))
			if (auto neighbor_pipe = std::dynamic_pointer_cast<Pipe>(neighbor))
				if (neighbor_pipe->directions[pipe_type].has(flipDirection(direction)))
					return neighbor_pipe;

		return nullptr;
	}

	void Pipe::updateTileID(PipeType pipe_type) {
		RealmPtr realm = getRealm();
		Tileset &tileset = realm->getTileset();
		const int8_t march_index = directions[pipe_type].getMarchIndex();
		tileIDs[pipe_type] = tileset[Corner(pipe_type)] + tileset.columnCount(realm->getGame()) * (march_index / 7) + march_index % 7;
	}

	void Pipe::tick(Game &game, float delta) {
		if (getSide() == Side::Server) {
			for (const PipeType pipe_type: PIPE_TYPES)
				if (auto network = networks[pipe_type])
					network->tick(game.currentTick);
			TileEntity::tick(game, delta);
		}
	}

	void Pipe::render(SpriteRenderer &sprite_renderer) {
		if (!isVisible())
			return;

		RealmPtr realm = getRealm();
		Tileset &tileset = realm->getTileset();

		Game &game = realm->getGame();

		const auto tilesize     = tileset.getTileSize();
		const auto texture      = tileset.getTexture(game);
		const auto column_count = tileset.columnCount(game);

		for (const PipeType pipe_type: reverse(PIPE_TYPES)) {
			std::optional<TileID> &tile_id = tileIDs[pipe_type];

			if (!tile_id && present[pipe_type])
				updateTileID(pipe_type);

			auto &extractors_corner = extractorsCorners[pipe_type];

			if (!extractors_corner)
				extractors_corner = tileset[ExtractorsCorner(pipe_type)];

			if (tile_id && *tile_id != 0) {
				const float x = (*tile_id % (texture->width / tilesize)) * tilesize;
				const float y = (*tile_id / (texture->width / tilesize)) * tilesize;
				sprite_renderer(*texture, {
					.x = float(position.column),
					.y = float(position.row),
					.xOffset = x / 2,
					.yOffset = y / 2,
					.sizeX = float(tilesize),
					.sizeY = float(tilesize),
				});
			}

			const auto [extractors_x, extractors_y] = extractors[pipe_type].extractorOffsets();
			if (extractors_x != 0 || extractors_y != 0) {
				const TileID extractor_tile = *extractors_corner + extractors_x + extractors_y * column_count;
				const float x = (extractor_tile % (texture->width / tilesize)) * tilesize;
				const float y = (extractor_tile / (texture->width / tilesize)) * tilesize;
				sprite_renderer(*texture, {
					.x = float(position.column),
					.y = float(position.row),
					.xOffset = x / 2,
					.yOffset = y / 2,
					.sizeX = float(tilesize),
					.sizeY = float(tilesize),
				});
			}
		}
	}

	void Pipe::onNeighborUpdated(Position offset) {
		const Direction direction{offset};

		if (direction == Direction::Invalid || offset.taxiDistance({}) != 1)
			return;

		const Position neighbor_position = position + offset;

		auto tile_entity = getRealm()->tileEntityAt(neighbor_position);

		if (!tile_entity)
			return;

		for (const PipeType pipe_type: PIPE_TYPES) {
			if (directions[pipe_type][direction]) {
				if (const auto &network = networks[pipe_type]; network && network->canWorkWith(tile_entity)) {
					if (extractors[pipe_type][direction])
						network->addExtraction(neighbor_position, flipDirection(direction));
					else
						network->addInsertion(neighbor_position, flipDirection(direction));
				}
			}
		}
	}

	void Pipe::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		buffer << directions;
		buffer << extractors;
		buffer << present;
	}

	void Pipe::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		buffer >> directions;
		buffer >> extractors;
		buffer >> present;
		tileIDs = {};
		extractorsCorners = {};
		loaded = {};
		networks = {};
	}

	void Pipe::setNetwork(PipeType pipe_type, const std::shared_ptr<PipeNetwork> &new_network) {
		networks[pipe_type] = new_network;
		loaded[pipe_type] = true;
	}

	std::shared_ptr<PipeNetwork> Pipe::getNetwork(PipeType pipe_type) const {
		return networks[pipe_type];
	}

	void Pipe::onSpawn() {
		TileEntity::onSpawn();

		auto realm = getRealm();
		auto shared = std::static_pointer_cast<Pipe>(shared_from_this());
		for (const PipeType pipe_type: PIPE_TYPES)
			realm->pipeLoader.floodFill(pipe_type, shared);
	}

	void Pipe::onRemove() {
		auto shared = std::static_pointer_cast<Pipe>(shared_from_this());
		for (const PipeType pipe_type: PIPE_TYPES)
			if (present[pipe_type])
				if (const auto &network = networks[pipe_type])
					network->removePipe(shared);

		TileEntity::onRemove();
	}

	bool Pipe::onInteractNextTo(const std::shared_ptr<Player> &, Modifiers) {
		for (const PipeType pipe_type: PIPE_TYPES) {
			if (auto network = networks[pipe_type]) {
				INFO(pipe_type << " network ID: " << network->getID() << ", loaded: " << std::boolalpha << loaded[pipe_type]);

				if (const auto &insertions = network->getInsertions(); !insertions.empty()) {
					INFO(pipe_type << " insertion points (" << insertions.size() << "):");
					for (const auto &[pos, direction]: insertions)
						INFO("- " << pos << ", " << direction);
				} else {
					INFO("No " << pipe_type << " insertion points.");
				}

				if (const auto &extractions = network->getExtractions(); !extractions.empty()) {
					INFO(pipe_type << " extraction points (" << extractions.size() << "):");
					for (const auto &[pos, direction]: extractions)
						INFO("- " << pos << ", " << direction);
				} else {
					INFO("No " << pipe_type << " extraction points.");
				}

				if (pipe_type == PipeType::Item)
					INFO("Overflow queue size: " << std::dynamic_pointer_cast<ItemNetwork>(network)->overflowCount());
				else if (pipe_type == PipeType::Energy)
					INFO("Stored energy: " << std::dynamic_pointer_cast<EnergyNetwork>(network)->energyContainer->energy);
			} else
				INFO("Pipe not connected to a(n) " << pipe_type << " network.");
		}

		return false;
	}

	void Pipe::toggle(PipeType pipe_type, Direction direction) {
		if (get(pipe_type, direction)) {
			set(pipe_type, direction, false);
			setExtractor(pipe_type, direction, false);
		} else
			set(pipe_type, direction, true);
	}

	void Pipe::toggleExtractor(PipeType pipe_type, Direction direction) {
		setExtractor(pipe_type, direction, !extractors[pipe_type][direction]);
	}

	void Pipe::setPresent(PipeType pipe_type, bool value) {
		present[pipe_type] = value;

		if (!value) {
			for (const Direction direction: ALL_DIRECTIONS) {
				set(pipe_type, direction, false);
				setExtractor(pipe_type, direction, false);
			}
		}
	}

	bool Pipe::reachable(PipeType pipe_type, const std::shared_ptr<Pipe> &target) {
		assert(target);
		auto shared = std::static_pointer_cast<Pipe>(shared_from_this());
		assert(shared);
		std::unordered_set visited{shared};
		std::deque queue{shared};

		while (!queue.empty()) {
			auto pipe = queue.front();
			queue.pop_front();
			visited.insert(pipe);

			if (pipe == target)
				return true;

			directions[pipe_type].iterate([&](Direction direction) {
				if (std::shared_ptr<Pipe> neighbor = pipe->getConnected(pipe_type, direction); neighbor && !visited.contains(neighbor))
					queue.push_back(neighbor);
			});
		}

		return false;
	}

	bool Pipe::get(PipeType pipe_type, Direction direction) {
		return directions[pipe_type][direction];
	}

	void Pipe::set(PipeType pipe_type, Direction direction, bool value) {
		std::shared_ptr<PipeNetwork> network = getNetwork(pipe_type);
		assert(network);

		if (value) {
			directions[pipe_type][direction] = true;

			if (!loaded[pipe_type])
				return;

			if (std::shared_ptr<Pipe> connection = getConnected(pipe_type, direction))
				network->absorb(connection->getNetwork(pipe_type));

			// If there's a tile entity at the attached position and it has an inventory, add it to the network as an insertion point.
			if (RealmPtr realm = weakRealm.lock())
				if (network->canWorkWith(realm->tileEntityAt(position + direction)))
					network->addInsertion(position + direction, flipDirection(direction));

			return;
		}

		if (!loaded[pipe_type]) {
			directions[pipe_type][direction] = false;
			return;
		}

		std::shared_ptr<Pipe> connection = getConnected(pipe_type, direction);
		directions[pipe_type][direction] = false;

		network->reconsiderInsertion(position + direction);

		if (connection && !reachable(pipe_type, connection))
			network->partition(connection);
	}

	void Pipe::setExtractor(PipeType pipe_type, Direction direction, bool value) {
		if (auto network = networks[pipe_type]) {
			extractors[pipe_type][direction] = value;

			if (value)
				network->addExtraction(position + direction, flipDirection(direction));
			else
				network->removeExtraction(position + direction, flipDirection(direction));

			network->reconsiderInsertion(position + direction);
			onNeighborUpdated(position + direction);
		} else {
			extractors[pipe_type][direction] = false;
		}
	}
}
