#include "game/Resource.h"
#include "game/ServerGame.h"
#include "game/Village.h"
#include "packet/VillageUpdatePacket.h"

#include "NameGen.h"

namespace Game3 {
	namespace {
		constexpr std::chrono::seconds PERIOD{1};

		constexpr auto getMultiplier() {
			return std::chrono::duration_cast<std::chrono::milliseconds>(PERIOD).count() / 1e6;
		}
	}

	Village::Village(ServerGame &game, const Place &place, const VillageOptions &options_):
		Village(game, place.realm->id, ChunkPosition(place.position), place.position, options_) {}

	Village::Village(ServerGame &game, RealmID realm_id, ChunkPosition chunk_position, const Position &position_, const VillageOptions &options_):
		Village(game, game.getNewVillageID(), realm_id, chunk_position, position_, options_) {}

	Village::Village(ServerGame &game, VillageID id_, RealmID realm_id, ChunkPosition chunk_position, const Position &position_, const VillageOptions &options_):
		HasGame(game.toServerPointer()),
		id(id_),
		name(NameGen::makeRandomLanguage(threadContext.rng).makeName()),
		realmID(realm_id),
		chunkPosition(chunk_position),
		position(position_),
		options(options_),
		richness(Richness::makeRandom(game)) {}

	Village::Village(VillageID id_, RealmID realm_id, std::string name_, ChunkPosition chunk_position, const Position &position_, const VillageOptions &options_, Richness richness_, Resources resources_):
		id(id_),
		name(std::move(name_)),
		realmID(realm_id),
		chunkPosition(chunk_position),
		position(position_),
		options(options_),
		richness(std::move(richness_)),
		resources(std::move(resources_)) {}

	std::optional<double> Village::getRichness(const Identifier &identifier) {
		return richness[identifier];
	}

	Tick Village::enqueueTick() {
		return getGame().enqueue(sigc::mem_fun(*this, &Village::tick));
	}

	void Village::addResources() {
		auto &registry = getGame().registry<ResourceRegistry>();

		for (const auto &[resource, value]: richness) {
			auto &in_map = resources[resource];
			in_map = std::min(registry.at(resource)->getCap(), in_map + value * getMultiplier());
		}
	}

	void Village::sendUpdates() {
		auto lock = subscribedPlayers.sharedLock();

		if (subscribedPlayers.empty())
			return;

		const VillageUpdatePacket packet(*this);

		for (const PlayerPtr &player: subscribedPlayers)
			player->send(packet);
	}

	void Village::tick(const TickArgs &) {
		addResources();
		sendUpdates();
		getGame().enqueue(sigc::mem_fun(*this, &Village::tick), PERIOD);
	}

	Game & Village::getGame() {
		return HasGame::getGame();
	}

	void Village::addSubscriber(PlayerPtr player) {
		auto lock = subscribedPlayers.uniqueLock();
		subscribedPlayers.insert(std::move(player));
	}

	void Village::removeSubscriber(const PlayerPtr &player) {
		auto lock = subscribedPlayers.uniqueLock();
		subscribedPlayers.erase(player);
	}

	std::string Village::getSQL() {
		return R"(
			CREATE TABLE IF NOT EXISTS villages (
				id INT8,
				realmID INT,
				chunkPosition VARCHAR(42),
				position VARCHAR(42),
				options VARCHAR(255),
				richness MEDIUMTEXT,
				resources MEDIUMTEXT,
				name VARCHAR(255),

				PRIMARY KEY(realmID, id)
			);
		)";
	}
}
