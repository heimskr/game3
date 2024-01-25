#include "biome/Biome.h"
#include "data/ConsumptionRule.h"
#include "data/ProductionRule.h"
#include "game/Resource.h"
#include "game/ServerGame.h"
#include "game/Village.h"
#include "packet/VillageUpdatePacket.h"
#include "util/Util.h"

#include "NameGen.h"

namespace Game3 {
	namespace {
		constexpr std::chrono::seconds PERIOD{30};

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
		richness(Richness::makeRandom(game)),
		resources(getDefaultResources()),
		randomValue(chooseRandomValue()) {}

	Village::Village(VillageID id_, RealmID realm_id, std::string name_, ChunkPosition chunk_position, const Position &position_, const VillageOptions &options_, Richness richness_, Resources resources_, LaborAmount labor_, double random_value):
		id(id_),
		name(std::move(name_)),
		realmID(realm_id),
		chunkPosition(chunk_position),
		position(position_),
		options(options_),
		richness(std::move(richness_)),
		resources(std::move(resources_)),
		labor(labor_),
		randomValue(random_value) {}

	std::optional<double> Village::getRichness(const Identifier &identifier) {
		return richness[identifier];
	}

	Tick Village::enqueueTick() {
		return getGame().enqueue(sigc::mem_fun(*this, &Village::tick));
	}

	void Village::produce(BiomeType biome, const ProductionRule &rule) {
		if (const auto &biomes = rule.getBiomes(); biomes && !biomes->contains(biome))
			return;

		auto lock = resources.uniqueLock();

		double multiplier = 1;

		if (auto effect = rule.getRichnessEffect()) {
			if (auto richness = getRichness(rule.getOutput().getID())) {
				multiplier = *effect * *richness;
				if (multiplier <= 0) {
					WARN("Can't produce " << rule.getOutput().getID() << ": multiplier too low (" << multiplier << ')');
					return;
				}
			} else {
				WARN("Can't produce " << rule.getOutput().getID() << ": no richness found");
				return;
			}
		}

		for (const ItemStack &stack: rule.getInputs()) {
			auto iter = resources.find(stack.getID());
			if (iter == resources.end() || iter->second < multiplier * stack.count)
				return;
		}

		double &output_count = resources[rule.getOutput().getID()];

		// Don't surpass the output cap.
		double add_count = rule.getOutput().count * multiplier;
		if (auto cap = rule.getCap(); cap && *cap < output_count + add_count) {
			double new_add = *cap - output_count;
			// TODO: is /= the right operation?
			multiplier /= add_count / new_add;
			add_count = new_add;
		}

		// Don't use too much labor.
		double labor_needed = rule.getLabor() * multiplier;
		if (labor < labor_needed) {
			add_count  *= labor / labor_needed;
			multiplier *= labor / labor_needed;
			labor_needed = labor;
		}

		for (const ItemStack &stack: rule.getInputs()) {
			resources.at(stack.getID()) -= multiplier * stack.count;
		}

		output_count += add_count;
	}

	void Village::produce(BiomeType biome, const ProductionRuleRegistry &rules) {
		for (const std::shared_ptr<ProductionRule> &rule: rules)
			produce(biome, *rule);
	}

	bool Village::consume(const ConsumptionRule &rule) {
		auto resources_lock = resources.uniqueLock();
		auto iter = resources.find(rule.getInput());

		if (iter == resources.end())
			return false;

		if (iter->second < 1.0)
			return false;

		--iter->second;
		labor += rule.getLaborOut();
		return true;
	}

	void Village::consume(const ConsumptionRuleRegistry &rules) {
		std::vector<std::shared_ptr<ConsumptionRule>> candidates;

		for (const auto &rule: rules) {
			if (rule->getAlways())
				consume(*rule);
			else
				candidates.push_back(rule);
		}

		if (!candidates.empty())
			consume(*choose(candidates, threadContext.rng));
	}

	void Village::tick(const TickArgs &args) {
		Game &game = args.game;

		BiomeType biome = Biome::VOID;
		if (std::optional<BiomeType> found_biome = game.getRealm(realmID)->tileProvider.copyBiomeType(position))
			biome = *found_biome;

		consume(game.registry<ConsumptionRuleRegistry>());
		produce(biome, game.registry<ProductionRuleRegistry>());
		sendUpdates();
		getGame().enqueue(sigc::mem_fun(*this, &Village::tick), PERIOD);
	}

	void Village::sendUpdates() {
		auto lock = subscribedPlayers.sharedLock();

		if (subscribedPlayers.empty())
			return;

		const VillageUpdatePacket packet(*this);

		for (const PlayerPtr &player: subscribedPlayers)
			player->send(packet);
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

	double Village::chooseRandomValue() {
		return std::uniform_real_distribution(0.0, 1.0)(threadContext.rng);
	}

	Resources Village::getDefaultResources() {
		return {
			{"base:item/iron_pickaxe", 2.0}
		};
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
				labor INT8,
				randomValue INT,

				PRIMARY KEY(realmID, id)
			);
		)";
	}
}
