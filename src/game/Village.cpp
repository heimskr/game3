#include "biome/Biome.h"
#include "data/ConsumptionRule.h"
#include "data/ProductionRule.h"
#include "game/Resource.h"
#include "game/Game.h"
#include "game/Village.h"
#include "packet/VillageUpdatePacket.h"
#include "threading/ThreadContext.h"
#include "util/Util.h"

#include "NameGen.h"

namespace Game3 {
	namespace {
		constexpr std::chrono::seconds PERIOD{1};

		constexpr auto getMultiplier() {
			return std::chrono::duration_cast<std::chrono::milliseconds>(PERIOD).count() / 60e3;
		}
	}

	Village::Village(Game &game, const Place &place, const VillageOptions &options_):
		Village(game, place.realm->id, ChunkPosition(place.position), place.position, options_) {}

	Village::Village(Game &game, RealmID realm_id, ChunkPosition chunk_position, const Position &position_, const VillageOptions &options_):
		Village(game, game.getNewVillageID(), realm_id, chunk_position, position_, options_) {}

	Village::Village(Game &game, VillageID id_, RealmID realm_id, ChunkPosition chunk_position, const Position &position_, const VillageOptions &options_):
		HasGame(game.shared_from_this()),
		id(id_),
		name(NameGen::makeRandomLanguage(threadContext.rng).makeName()),
		realmID(realm_id),
		chunkPosition(chunk_position),
		position(position_),
		options(options_),
		richness(Richness::makeRandom(game)),
		resources(getDefaultResources()),
		randomValue(chooseRandomValue()),
		greed(chooseGreed()) {}

	Village::Village(VillageID id_, RealmID realm_id, std::string name_, ChunkPosition chunk_position, const Position &position_, const VillageOptions &options_, Richness richness_, Resources resources_, LaborAmount labor_, double random_value, double greed_):
		id(id_),
		name(std::move(name_)),
		realmID(realm_id),
		chunkPosition(chunk_position),
		position(position_),
		options(options_),
		richness(std::move(richness_)),
		resources(std::move(resources_)),
		labor(labor_),
		randomValue(random_value),
		greed(greed_) {}

	std::optional<double> Village::getRichness(const Identifier &identifier) const {
		return richness[identifier];
	}

	std::optional<double> Village::getResourceAmount(const Identifier &resource) const {
		auto lock = resources.sharedLock();
		if (auto iter = resources.find(resource); iter != resources.end())
			return iter->second;
		return std::nullopt;
	}

	void Village::setResourceAmount(const Identifier &resource, double amount) {
		{
			auto lock = resources.uniqueLock();
			if (std::abs(amount) < 0.00001)
				resources.erase(resource);
			else
				resources[resource] = amount;
		}
		sendUpdates();
	}

	Tick Village::enqueueTick() {
		GamePtr game = getGame();
		return game->enqueue(sigc::mem_fun(*this, &Village::tick));
	}

	void Village::produce(BiomeType biome, const ProductionRule &rule) {
		if (const auto &biomes = rule.getBiomes(); biomes && !biomes->contains(biome))
			return;

		auto lock = resources.uniqueLock();

		double multiplier = getMultiplier();

		if (auto effect = rule.getRichnessEffect()) {
			if (auto richness = getRichness(rule.getOutput().getID())) {
				multiplier = *effect * *richness;
				if (multiplier <= 0)
					return;
			} else {
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

		if (multiplier == 0)
			return;

		for (const ItemStack &stack: rule.getInputs())
			resources.at(stack.getID()) -= multiplier * stack.count;

		output_count += add_count;
		labor -= labor_needed;
	}

	void Village::produce(BiomeType biome, const ProductionRuleRegistry &rules) {
		for (const std::shared_ptr<ProductionRule> &rule: rules)
			produce(biome, *rule);
	}

	bool Village::consume(const ConsumptionRule &rule) {
		const double rate = rule.getRate();
		auto resources_lock = resources.uniqueLock();
		auto iter = resources.find(rule.getInput());

		if (iter == resources.end())
			return false;

		double &amount = iter->second;

		if (amount < rate)
			return false;

		const auto [min, max] = rule.getLaborRange();

		const bool in_range = min <= labor && labor <= max;

		if (rule.getIgnoreLabor()) {
			if (in_range)
				labor += rule.getLaborOut() * rate;
		} else {
			if (!in_range)
				return false;
			labor += rule.getLaborOut() * rate;
		}

		amount -= rate;
		if (amount < 0.0001)
			resources.erase(iter);

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
		const GamePtr &game = args.game;

		BiomeType biome = Biome::VOID;
		if (std::optional<BiomeType> found_biome = game->getRealm(realmID)->tileProvider.copyBiomeType(position))
			biome = *found_biome;

		consume(game->registry<ConsumptionRuleRegistry>());
		produce(biome, game->registry<ProductionRuleRegistry>());
		sendUpdates();
		game->enqueue(sigc::mem_fun(*this, &Village::tick), PERIOD);
	}

	void Village::sendUpdates() {
		auto lock = subscribedPlayers.sharedLock();

		if (subscribedPlayers.empty())
			return;

		const VillageUpdatePacket packet(*this);

		for (const PlayerPtr &player: subscribedPlayers)
			player->send(packet);
	}

	GamePtr Village::getGame() const {
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

	size_t Village::getSubscriberCount() const {
		auto lock = subscribedPlayers.sharedLock();
		return subscribedPlayers.size();
	}

	double Village::chooseRandomValue() {
		return std::uniform_real_distribution(0.0, 1.0)(threadContext.rng);
	}

	double Village::chooseGreed() {
		return std::uniform_real_distribution(0.2, 1.0)(threadContext.rng);
	}

	Resources Village::getDefaultResources() {
		return {
			{"base:item/meat", 10}
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
				randomValue DOUBLE,
				greed DOUBLE,

				PRIMARY KEY(realmID, id)
			);
		)";
	}
}
