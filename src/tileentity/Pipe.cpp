#include "util/Log.h"
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
	Pipe::Pipe():
		Pipe(Position(-1, -1)) {}

	Pipe::Pipe(Position position_):
		TileEntity("base:tile/missing"_id, ID(), position_, false) {}

	Identifier Pipe::Corner(Substance type) {
		switch (type) {
			case Substance::Item:   return ItemCorner();
			case Substance::Fluid:  return FluidCorner();
			case Substance::Energy: return EnergyCorner();
			case Substance::Data:   return DataCorner();
			default: return {};
		}
	}

	Identifier Pipe::ExtractorsCorner(Substance type) {
		switch (type) {
			case Substance::Item:   return ItemExtractorsCorner();
			case Substance::Fluid:  return FluidExtractorsCorner();
			case Substance::Energy: return EnergyExtractorsCorner();
			case Substance::Data:   return DataExtractorsCorner();
			default: return {};
		}
	}

	DirectionalContainer<std::shared_ptr<Pipe>> Pipe::getConnected(Substance pipe_type) const {
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

	std::shared_ptr<Pipe> Pipe::getConnected(Substance pipe_type, Direction direction) const {
		auto realm = getRealm();

		if (auto neighbor = realm->tileEntityAt(position + direction))
			if (auto neighbor_pipe = std::dynamic_pointer_cast<Pipe>(neighbor))
				if (neighbor_pipe->directions[pipe_type].has(flipDirection(direction)))
					return neighbor_pipe;

		return nullptr;
	}

	void Pipe::updateTileID(Substance pipe_type) {
		RealmPtr realm = getRealm();
		Tileset &tileset = realm->getTileset();
		const int8_t march_index = directions[pipe_type].getMarchIndex();
		tileIDs[pipe_type] = tileset[Corner(pipe_type)] + march_index;
	}

	void Pipe::tick(const TickArgs &args) {
		if (getSide() == Side::Server) {
			for (const Substance pipe_type: PIPE_TYPES)
				if (auto network = networks[pipe_type])
					network->tick(args.game, args.game->getCurrentTick());
			TileEntity::tick(args);
		}
	}

	void Pipe::render(SpriteRenderer &sprite_renderer) {
		if (!isVisible())
			return;

		RealmPtr realm = getRealm();
		Tileset &tileset = realm->getTileset();

		GamePtr game = realm->getGame();

		const auto tilesize = tileset.getTileSize();
		const auto texture  = tileset.getTexture(*game);

		for (const Substance pipe_type: reverse(PIPE_TYPES)) {
			std::optional<TileID> &tile_id = tileIDs[pipe_type];

			if (!tile_id && present[pipe_type])
				updateTileID(pipe_type);

			auto &extractors_corner = extractorsCorners[pipe_type];

			if (!extractors_corner)
				extractors_corner = tileset[ExtractorsCorner(pipe_type)];

			if (tile_id && *tile_id != 0) {
				const float x = (*tile_id % (texture->width / tilesize)) * tilesize;
				const float y = (*tile_id / (texture->width / tilesize)) * tilesize;
				sprite_renderer(texture, {
					.x = float(position.column),
					.y = float(position.row),
					.offsetX = x / 2,
					.offsetY = y / 2,
					.sizeX = float(tilesize),
					.sizeY = float(tilesize),
				});
			}

			if (const auto extractors_march = extractors[pipe_type].getMarchIndex()) {
				const TileID extractor_tile = *extractors_corner + extractors_march;
				const float x = (extractor_tile % (texture->width / tilesize)) * tilesize;
				const float y = (extractor_tile / (texture->width / tilesize)) * tilesize;
				sprite_renderer(texture, {
					.x = float(position.column),
					.y = float(position.row),
					.offsetX = x / 2,
					.offsetY = y / 2,
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

		TileEntityPtr tile_entity = getRealm()->tileEntityAt(neighbor_position);

		if (!tile_entity)
			return;

		for (const Substance pipe_type: PIPE_TYPES) {
			if (directions[pipe_type][direction]) {
				if (const std::shared_ptr<PipeNetwork> &network = networks[pipe_type]; network && network->canWorkWith(tile_entity)) {
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
		buffer << itemFilters;
	}

	void Pipe::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		buffer >> directions;
		buffer >> extractors;
		buffer >> present;
		buffer >> itemFilters;
		tileIDs = {};
		extractorsCorners = {};
		loaded = {};
		networks = {};
	}

	void Pipe::setNetwork(Substance pipe_type, const std::shared_ptr<PipeNetwork> &new_network) {
		networks[pipe_type] = new_network;
		loaded[pipe_type] = true;
	}

	std::shared_ptr<PipeNetwork> Pipe::getNetwork(Substance pipe_type) const {
		return networks[pipe_type];
	}

	void Pipe::onSpawn() {
		TileEntity::onSpawn();

		auto realm = getRealm();
		auto shared = std::static_pointer_cast<Pipe>(shared_from_this());
		for (const Substance pipe_type: PIPE_TYPES)
			realm->pipeLoader.floodFill(pipe_type, shared);
	}

	void Pipe::onRemove() {
		auto shared = std::static_pointer_cast<Pipe>(shared_from_this());
		for (const Substance pipe_type: PIPE_TYPES)
			if (present[pipe_type])
				if (const auto &network = networks[pipe_type])
					network->removePipe(shared);

		TileEntity::onRemove();
	}

	bool Pipe::onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &/*used_item*/, Hand) {
		// if (!used_item || used_item->item->identifier != "base:item/wrench")
		// 	return false;

		bool first = true;

		for (const Substance pipe_type: PIPE_TYPES) {
			if (auto network = networks[pipe_type]) {
				if (first) {
					first = false;
					INFO("================================");
				} else {
					INFO("--------------------------------");
				}

				INFO("{} network ID: {}, loaded: {}", pipe_type, network->getID(), loaded[pipe_type]);

				if (const auto &insertions = network->getInsertions(); !insertions.empty()) {
					INFO("{} insertion points ({}):", pipe_type, insertions.size());
					for (const auto &[pos, direction]: insertions)
						INFO("- {}, {}", pos, direction);
				} else {
					INFO("No {} insertion points.", pipe_type);
				}

				if (const auto &extractions = network->getExtractions(); !extractions.empty()) {
					INFO("{} extraction points ({}):", pipe_type, extractions.size());
					for (const auto &[pos, direction]: extractions)
						INFO("- {}, {}", pos, direction);
				} else {
					INFO("No {} extraction points.", pipe_type);
				}

				if (pipe_type == Substance::Item)
					INFO("Overflow queue size: {}", std::dynamic_pointer_cast<ItemNetwork>(network)->overflowCount());
				else if (pipe_type == Substance::Energy)
					INFO("Stored energy: {}", std::dynamic_pointer_cast<EnergyNetwork>(network)->energyContainer->energy);
			} else
				INFO("Pipe not connected to a(n) {} network.", pipe_type);
		}

		return false;
	}

	void Pipe::autopipe(Substance pipe_type) {
		RealmPtr realm = getRealm();
		std::shared_ptr<PipeNetwork> network = getNetwork(pipe_type);

		for (const Direction direction: ALL_DIRECTIONS) {
			TileEntityPtr neighbor = realm->tileEntityAt(position + direction);
			if (!neighbor)
				continue;

			bool new_value = false;

			if (auto pipe = std::dynamic_pointer_cast<Pipe>(neighbor)) {
				new_value = pipe->getPresent(pipe_type);
				if (new_value) {
					Direction flipped = flipDirection(direction);
					if (!pipe->get(pipe_type, flipped)) {
						pipe->set(pipe_type, flipped, true);
						pipe->increaseUpdateCounter();
						pipe->queueBroadcast();
					}
				}
			} else if (network->canWorkWith(neighbor)) {
				new_value = true;
			}

			set(pipe_type, direction, new_value);
		}

		increaseUpdateCounter();
		queueBroadcast();
	}

	void Pipe::toggle(Substance pipe_type, Direction direction) {
		if (get(pipe_type, direction)) {
			set(pipe_type, direction, false);
			setExtractor(pipe_type, direction, false);
		} else
			set(pipe_type, direction, true);
	}

	void Pipe::toggleExtractor(Substance pipe_type, Direction direction) {
		setExtractor(pipe_type, direction, !extractors[pipe_type][direction]);
	}

	void Pipe::setPresent(Substance pipe_type, bool value) {
		present[pipe_type] = value;

		if (!value) {
			for (const Direction direction: ALL_DIRECTIONS) {
				set(pipe_type, direction, false);
				setExtractor(pipe_type, direction, false);
			}
		}
	}

	std::pair<std::shared_ptr<Pipe>, std::shared_ptr<PipeNetwork>> Pipe::getNeighbor(Substance type, Direction direction) const {
		TileEntityPtr tile_entity = getRealm()->tileEntityAt(position + direction);
		auto neighbor = std::dynamic_pointer_cast<Pipe>(tile_entity);
		if (!neighbor)
			return {};

		return {neighbor, neighbor->getNetwork(type)};
	}

	bool Pipe::reachable(Substance pipe_type, const std::shared_ptr<Pipe> &target) {
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
				if (std::shared_ptr<Pipe> neighbor = pipe->getConnected(pipe_type, direction); neighbor && !neighbor->dying[pipe_type] && !visited.contains(neighbor))
					queue.push_back(neighbor);
			});
		}

		return false;
	}

	bool Pipe::get(Substance pipe_type, Direction direction) {
		return directions[pipe_type][direction];
	}

	void Pipe::set(Substance pipe_type, Direction direction, bool value) {
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

		network->reconsiderPoints(position + direction);

		if (connection && !reachable(pipe_type, connection)) {
			std::shared_ptr<PipeNetwork> new_network = network->partition(connection);
			{
				const auto &insertions = new_network->getInsertions();
				auto lock = insertions.sharedLock();
				for (const auto &[insertion_position, insertion_direction]: insertions)
					network->removeInsertion(insertion_position, insertion_direction);
			}
			const auto &extractions = new_network->getExtractions();
			auto lock = extractions.sharedLock();
			for (const auto &[extraction_position, extraction_direction]: extractions)
				network->removeExtraction(extraction_position, extraction_direction);
		}
	}

	void Pipe::setExtractor(Substance pipe_type, Direction direction, bool value) {
		if (std::shared_ptr<PipeNetwork> network = networks[pipe_type]) {
			extractors[pipe_type][direction] = value;

			if (value)
				network->addExtraction(position + direction, flipDirection(direction));
			else
				network->removeExtraction(position + direction, flipDirection(direction));

			network->reconsiderPoints(position + direction);
			onNeighborUpdated(position + direction);
		} else {
			extractors[pipe_type][direction] = false;
		}
	}
}
