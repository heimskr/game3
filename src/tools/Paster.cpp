#include "entity/ServerPlayer.h"
#include "game/Game.h"
#include "tools/Paster.h"
#include "types/Position.h"
#include "realm/Realm.h"
#include "util/Cast.h"
#include "util/Util.h"

namespace Game3 {
	class RemoteClient;

	Paster::Paster(std::string_view string) {
		ingest(string);
	}

	void Paster::ingest(std::string_view string) {
		identifiers.clear();
		tiles.clear();
		tileEntityJSON.clear();

		std::vector<std::string_view> lines = split(string, "\n", true);
		if (lines.empty())
			throw std::invalid_argument("Invalid Paster input: no lines");

		std::vector<std::string_view> slashes = split(lines[0], "/", true);
		if (slashes.empty())
			throw std::invalid_argument("Invalid Paster input: no slashes");

		for (const std::string_view &identifier_string: split(slashes[0], ";", false)) {
			if (identifier_string.find(':') == std::string_view::npos)
				identifiers.emplace_back("base:tile/" + std::string(identifier_string));
			else
				identifiers.emplace_back(identifier_string);
		}

		for (size_t i = 1; i < slashes.size(); ++i) {
			std::string_view segment = slashes[i];

			const size_t equals = segment.find('=');

			if (equals == std::string_view::npos)
				throw std::invalid_argument("Invalid Paster input: tile segment missing '='");

			Position anchor;

			{
				std::string_view position_string = segment.substr(0, equals);
				const size_t comma = position_string.find(',');
				if (comma == std::string_view::npos)
					throw std::invalid_argument("Invalid Paster input: tile position missing ','");
				anchor.row = parseNumber<Index>(position_string.substr(0, comma));
				anchor.column = parseNumber<Index>(position_string.substr(comma + 1));
			}

			std::array<Identifier *, LAYER_COUNT> layers{};

			for (std::string_view bundle: split(segment.substr(equals + 1), ",", false)) {
				if (!bundle.empty()) {
					std::vector<std::string_view> indices = split(bundle, ":", false);

					size_t j = 0;

					for (; j < indices.size() && j < LAYER_COUNT; ++j)
						layers[j] = &identifiers.at(parseNumber<size_t>(indices[j]));

					for (; j < LAYER_COUNT; ++j)
						layers[j] = &identifiers.at(0);
				}

				tiles.emplace(anchor, layers);
				++anchor.column;
			}
		}

		for (size_t i = 1; i < lines.size(); ++i) {
			std::string_view line = lines[i];

			const size_t equals = line.find('=');

			if (equals == std::string_view::npos)
				throw std::invalid_argument("Invalid Paster input: tile entity line missing '='");

			Position anchor;

			{
				std::string_view position_string = line.substr(0, equals);
				const size_t comma = position_string.find(',');
				if (comma == std::string_view::npos)
					throw std::invalid_argument("Invalid Paster input: tile entity position missing ','");
				anchor.row = parseNumber<Index>(position_string.substr(0, comma));
				anchor.column = parseNumber<Index>(position_string.substr(comma + 1));
			}

			tileEntityJSON.emplace(anchor, nlohmann::json::parse(line.substr(equals + 1)));
		}
	}

	void Paster::paste(const RealmPtr &realm, const Position &anchor) {
		GamePtr game = realm->getGame();
		assert(game->getSide() == Side::Server);

		std::unordered_set<ChunkPosition> chunks;

		{
			auto pauser = realm->guardGeneration();

			for (const auto &[position, layers]: tiles) {
				const Position adjusted = anchor + position;
				chunks.insert(adjusted.getChunk());
				for (Layer layer: allLayers) {
					realm->setFluid(adjusted, FluidTile{});
					realm->setTile(layer, adjusted, *layers.at(getIndex(layer)), true);
				}
			}

			for (const auto &[position, json]: tileEntityJSON) {
				TileEntityPtr tile_entity = TileEntity::fromJSON(game, json);
				tile_entity->position = anchor + position;
				tile_entity->setRealm(realm);
				tile_entity->init(*game);
				realm->addToMaps(tile_entity);
				realm->attach(tile_entity);
				tile_entity->onSpawn();
			}
		}

		TileProvider &provider = realm->tileProvider;

		for (ChunkPosition chunk: chunks)
			provider.updateChunk(chunk);

		std::unordered_set<std::shared_ptr<RemoteClient>> clients;

		{
			const auto &player_set = realm->getPlayers();
			auto lock = player_set.sharedLock();
			for (const auto &weak_player: player_set) {
				if (PlayerPtr player = weak_player.lock()) {
					for (ChunkPosition chunk: chunks) {
						if (player->canSee(realm->getID(), chunk.topLeft())) {
							clients.insert(std::static_pointer_cast<ServerPlayer>(player)->getClient());
							break;
						}
					}
				}
			}
		}

		for (const auto &client: clients)
			realm->sendTo(*client);
	}
}
