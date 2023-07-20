#include "Log.h"
#include "Tileset.h"
#include "pipes/PipeNetwork.h"
#include "realm/Realm.h"
#include "tileentity/Pipe.h"
#include "ui/SpriteRenderer.h"
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

		for (const Direction direction: directions[pipe_type].toVector())
			if (auto neighbor = realm->tileEntityAt(position + direction))
				if (auto neighbor_pipe = neighbor->cast<Pipe>())
					if (neighbor_pipe->directions[pipe_type].has(flipDirection(direction)))
						out[direction] = neighbor_pipe;

		return out;
	}

	std::shared_ptr<Pipe> Pipe::getConnected(PipeType pipe_type, Direction direction) const {
		auto realm = getRealm();

		if (auto neighbor = realm->tileEntityAt(position + direction))
			if (auto neighbor_pipe = neighbor->cast<Pipe>())
				if (neighbor_pipe->directions[pipe_type].has(flipDirection(direction)))
					return neighbor_pipe;

		return nullptr;
	}

	void Pipe::updateTileID(PipeType pipe_type) {
		RealmPtr realm = getRealm();
		Tileset &tileset = realm->getTileset();
		const auto march_index = directions[pipe_type].getMarchIndex();
		tileIDs[pipe_type] = tileset[Corner(pipe_type)] + tileset.columnCount(realm->getGame()) * (march_index / 7) + march_index % 7;
	}

	void Pipe::render(SpriteRenderer &sprite_renderer) {
		RealmPtr realm = getRealm();
		Tileset &tileset = realm->getTileset();

		Game &game = realm->getGame();

		const auto tilesize     = tileset.getTileSize();
		const auto texture      = tileset.getTexture(game);
		const auto column_count = tileset.columnCount(game);

		for (const PipeType pipe_type: reverse(PIPE_TYPES)) {
			if (!tileIDs[pipe_type] && present[pipe_type])
				updateTileID(pipe_type);

			auto &extractors_corner = extractorsCorners[pipe_type];

			if (!extractors_corner)
				extractors_corner = tileset[ExtractorsCorner(pipe_type)];

			std::optional<TileID> &tile_id = tileIDs[pipe_type];

			if (tile_id && *tile_id != 0) {
				const float x = (*tile_id % (*texture->width / tilesize)) * tilesize;
				const float y = (*tile_id / (*texture->width / tilesize)) * tilesize;
				sprite_renderer(*texture, {
					.x = static_cast<float>(position.column),
					.y = static_cast<float>(position.row),
					.x_offset = x / 2,
					.y_offset = y / 2,
					.size_x = static_cast<float>(tilesize),
					.size_y = static_cast<float>(tilesize),
				});
			}

			const auto [extractors_x, extractors_y] = extractors[pipe_type].extractorOffsets();
			if (extractors_x != 0 || extractors_y != 0) {
				const TileID extractor_tile = *extractors_corner + extractors_x + extractors_y * column_count;
				const float x = (extractor_tile % (*texture->width / tilesize)) * tilesize;
				const float y = (extractor_tile / (*texture->width / tilesize)) * tilesize;
				sprite_renderer(*texture, {
					.x = static_cast<float>(position.column),
					.y = static_cast<float>(position.row),
					.x_offset = x / 2,
					.y_offset = y / 2,
					.size_x = static_cast<float>(tilesize),
					.size_y = static_cast<float>(tilesize),
				});
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
		auto realm = getRealm();
		auto shared = shared_from_this()->cast<Pipe>();
		for (const PipeType pipe_type: PIPE_TYPES)
			realm->pipeLoader.floodFill(pipe_type, shared);
		TileEntity::onSpawn();
	}

	bool Pipe::onInteractNextTo(const std::shared_ptr<Player> &) {
		if (auto network = networks[PipeType::Item])
			INFO("Item network ID: " << network->getID() << ", loaded: " << std::boolalpha << loaded[PipeType::Item]);
		else
			INFO("Pipe not connected to an item network.");

		if (auto network = networks[PipeType::Fluid])
			INFO("Fluid network ID: " << network->getID() << ", loaded: " << std::boolalpha << loaded[PipeType::Fluid]);
		else
			INFO("Pipe not connected to a fluid network.");

		if (auto network = networks[PipeType::Energy])
			INFO("Energy network ID: " << network->getID() << ", loaded: " << std::boolalpha << loaded[PipeType::Energy]);
		else
			INFO("Pipe not connected to an energy network.");

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
			for (const Direction direction: {Direction::Up, Direction::Right, Direction::Down, Direction::Left}) {
				set(pipe_type, direction, false);
				setExtractor(pipe_type, direction, false);
			}
		}
	}

	bool Pipe::reachable(PipeType pipe_type, const std::shared_ptr<Pipe> &target) {
		assert(target);
		std::shared_ptr<Pipe> shared = shared_from_this()->cast<Pipe>();
		assert(shared);
		std::unordered_set visited{shared};
		std::deque queue{shared};

		while (!queue.empty()) {
			auto pipe = queue.front();
			queue.pop_front();
			visited.insert(pipe);

			if (pipe == target)
				return true;

			for (const Direction direction: {Direction::Up, Direction::Right, Direction::Down, Direction::Left})
				if (auto neighbor = pipe->getConnected(pipe_type, direction); neighbor && !visited.contains(neighbor))
					queue.push_back(neighbor);
		}

		return false;
	}

	bool Pipe::get(PipeType pipe_type, Direction direction) {
		return directions[pipe_type][direction];
	}

	void Pipe::set(PipeType pipe_type, Direction direction, bool value) {
		if (value) {
			directions[pipe_type][direction] = true;

			if (!loaded[pipe_type])
				return;

			if (std::shared_ptr<Pipe> connection = getConnected(pipe_type, direction)) {
				std::shared_ptr<PipeNetwork> network = getNetwork(pipe_type);
				assert(network);
				network->absorb(connection->getNetwork(pipe_type));
			}

			return;
		}


		if (!loaded[pipe_type]) {
			directions[pipe_type][direction] = false;
			return;
		}

		std::shared_ptr<Pipe> connection = getConnected(pipe_type, direction);
		directions[pipe_type][direction] = false;

		if (connection && !reachable(pipe_type, connection)) {
			std::shared_ptr<PipeNetwork> network = getNetwork(pipe_type);
			assert(network);
			network->partition(connection);
		}
	}

	void Pipe::setExtractor(PipeType pipe_type, Direction direction, bool value) {
		if (auto network = networks[pipe_type]) {
			if (value) {
				network->addExtraction(position + direction, flipDirection(direction));
			} else {
				network->removeExtraction(position + direction, flipDirection(direction));
			}
		}
	}
}
