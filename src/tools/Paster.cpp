#include "entity/ServerPlayer.h"
#include "tools/Paster.h"
#include "types/Position.h"
#include "realm/Realm.h"
#include "util/Cast.h"
#include "util/Util.h"

namespace Game3 {
	class RemoteClient;

	Paster::Paster(std::string_view string) {
		std::vector<std::string_view> lines = split(string, "\n", true);
		if (lines.empty())
			throw std::invalid_argument("Invalid Paster input: no lines");

		for (const std::string_view &identifier_string: split(lines[0], ";", false)) {
			if (identifier_string.find(':') == std::string_view::npos)
				identifiers.emplace_back("base:tile/" + std::string(identifier_string));
			else
				identifiers.emplace_back(identifier_string);
		}

		for (size_t i = 1; i < lines.size(); ++i) {
			std::string_view line = lines[i];

			size_t equals = line.find('=');

			if (equals == std::string_view::npos)
				throw std::invalid_argument("Invalid Paster input: position line missing '='");

			Position anchor;

			{
				std::string_view position_string = line.substr(0, equals);
				size_t comma = position_string.find(',');
				if (comma == std::string_view::npos)
					throw std::invalid_argument("Invalid Paster input: position missing ','");
				anchor.row = parseNumber<Index>(position_string.substr(0, comma));
				anchor.column = parseNumber<Index>(position_string.substr(comma + 1));
			}

			std::array<Identifier *, LAYER_COUNT> layers{};

			for (std::string_view bundle: split(line.substr(equals + 1), ",", false)) {
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
	}

	void Paster::paste(Realm &realm, const Position &anchor) {
		assert(realm.getSide() == Side::Server);

		std::unordered_set<ChunkPosition> chunks;

		{
			auto pauser = realm.guardGeneration();
			for (const auto &[position, layers]: tiles) {
				const Position adjusted = anchor + position;
				chunks.insert(adjusted.getChunk());
				for (Layer layer: allLayers) {
					realm.setFluid(adjusted, FluidTile{});
					realm.setTile(layer, adjusted, *layers.at(getIndex(layer)), true);
				}
			}
		}

		TileProvider &provider = realm.tileProvider;

		for (ChunkPosition chunk: chunks)
			provider.updateChunk(chunk);

		std::unordered_set<std::shared_ptr<RemoteClient>> clients;

		{
			const auto &player_set = realm.getPlayers();
			auto lock = player_set.sharedLock();
			for (const auto &weak_player: player_set) {
				if (PlayerPtr player = weak_player.lock()) {
					for (ChunkPosition chunk: chunks) {
						if (player->canSee(realm.getID(), chunk.topLeft())) {
							clients.insert(std::static_pointer_cast<ServerPlayer>(player)->getClient());
							break;
						}
					}
				}
			}
		}

		for (const auto &client: clients)
			realm.sendTo(*client);
	}
}
