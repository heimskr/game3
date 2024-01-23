#include "game/ServerGame.h"
#include "game/Village.h"

namespace Game3 {
	namespace {
		constexpr std::chrono::seconds PERIOD{1};
	}

	Village::Village(ServerGame &game, const Place &place, const VillageOptions &options_):
		Village(game, place.realm->id, ChunkPosition(place.position), place.position, options_) {}

	Village::Village(ServerGame &game, RealmID realm_id, ChunkPosition chunk_position, const Position &position_, const VillageOptions &options_):
		Village(game, game.getNewVillageID(), realm_id, chunk_position, position_, options_) {}

	Village::Village(ServerGame &game, size_t id_, RealmID realm_id, ChunkPosition chunk_position, const Position &position_, const VillageOptions &options_):
		HasGame(game.toServerPointer()),
		id(id_),
		realmID(realm_id),
		chunkPosition(chunk_position),
		position(position_),
		options(options_),
		richness(Richness::makeRandom(game)) {}

	Village::Village(size_t id_, RealmID realm_id, ChunkPosition chunk_position, const Position &position_, const VillageOptions &options_, Richness richness_, Resources resources_):
		id(id_),
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

	void Village::tick(const TickArgs &) {
		getGame().enqueue(sigc::mem_fun(*this, &Village::tick), PERIOD);
	}

	Game & Village::getGame() {
		return HasGame::getGame();
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

				PRIMARY KEY(realmID, id)
			);
		)";
	}
}
