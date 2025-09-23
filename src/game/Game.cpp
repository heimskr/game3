#include "algorithm/AStar.h"
#include "fluid/Fluid.h"
#include "game/ClientGame.h"
#include "game/Game.h"
#include "game/InteractionSet.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "graphics/Tileset.h"
#include "lib/JSON.h"
#include "ui/module/AutocrafterModule.h"
#include "ui/module/ChemicalReactorModule.h"
#include "ui/module/CombinerModule.h"
#include "ui/module/EnergyModule.h"
#include "ui/module/FluidsModule.h"
#include "ui/module/GeneInfoModule.h"
#include "ui/module/InventoryModule.h"
#include "ui/module/ItemFiltersModule.h"
#include "ui/module/MicroscopeModule.h"
#include "ui/module/ModuleFactory.h"
#include "ui/module/MultiModule.h"
#include "ui/module/MutatorModule.h"
#include "ui/module/RadiusMachineModule.h"
#include "ui/module/TextModule.h"
#include "ui/module/VillageTradeModule.h"
#include "util/FS.h"
#include "util/Timer.h"
#include "util/Util.h"

namespace Game3 {
	namespace {
		size_t DEFAULT_GAME_POOL_SIZE = 4;
	}

	Game::Game():
		Game(DEFAULT_GAME_POOL_SIZE) {}

	Game::Game(size_t pool_size):
		pool(pool_size) {
			pool.start();
		}

	Game::~Game() {
		dying = true;
	}

	bool Game::tick() {
		auto now = getTime();
		auto difference = now - lastTime;
		lastTime = now;
		delta = std::chrono::duration_cast<std::chrono::nanoseconds>(difference).count() / 1e9;
		time = time + delta;
		HasTickQueue::tick(delta, TickArgs{shared_from_this(), getCurrentTick(), delta});
		return true;
	}

	void Game::addModuleFactories() {
		add(ModuleFactory::create<AutocrafterModule>());
		add(ModuleFactory::create<ChemicalReactorModule>());
		add(ModuleFactory::create<CombinerModule>());
		add(ModuleFactory::create<EnergyModule>());
		add(ModuleFactory::create<FluidsModule>());
		add(ModuleFactory::create<InventoryModule>());
		add(ModuleFactory::create<ItemFiltersModule>());
		add(ModuleFactory::create<MicroscopeModule<0>>());
		add(ModuleFactory::create<MicroscopeModule<1, Substance::Energy>>());
		add(ModuleFactory::create<MicroscopeModule<1, Substance::Energy, Substance::Fluid>>());
		add(ModuleFactory::create<MicroscopeModule<2, Substance::Energy>>());
		add(ModuleFactory::create<MultiModule<Substance::Item, Substance::Energy, Substance::Fluid>>());
		add(ModuleFactory::create<MultiModule<Substance::Item, Substance::Fluid>>());
		add(ModuleFactory::create<MultiModule<Substance::Item, Substance::Energy>>());
		add(ModuleFactory::create<MultiModule<Substance::Energy, Substance::Fluid>>());
		add(ModuleFactory::create<MutatorModule>());
		add(ModuleFactory::create<RadiusMachineModule>());
		add(ModuleFactory::create<TextModule>());
		add(ModuleFactory::create<VillageTradeModule>());
#ifdef GAME3_ENABLE_SCRIPTING
		// add(GTKModuleFactory::create<ComputerModule>());
#endif
	}

	void Game::initialSetup(const std::filesystem::path &dir) {
		initRegistries();
		addItems();
		traverseData(dataRoot / dir);
		addRealms();
		addEntityFactories();
		addTileEntityFactories();
		addPacketFactories();
		addLocalCommandFactories();
		addTiles();
		addModuleFactories();
		addMinigameFactories();
		addFluids();
		addStatusEffectFactories();
	}

	void Game::initEntities() {
		for (const auto &[realm_id, realm]: realms) {
			realm->initEntities();
		}
	}

	void Game::initInteractionSets() {
		interactionSets.clear();
		auto standard = std::make_shared<StandardInteractions>();
		for (const auto &type: registry<RealmTypeRegistry>().items) {
			interactionSets.emplace(type, standard);
		}
	}

	void Game::add(std::shared_ptr<Item> item) {
		itemRegistry->add(item->identifier, item);
		for (const auto &attribute: item->attributes) {
			itemsByAttribute[attribute].insert(item);
		}
	}

	void Game::add(ModuleFactory &&factory) {
		auto shared = std::make_shared<ModuleFactory>(std::move(factory));
		registry<ModuleFactoryRegistry>().add(shared->identifier, shared);
	}

	void Game::addRecipe(const boost::json::value &json) {
		Identifier identifier(std::string_view(json.at(0).as_string()));
		if (identifier.getPathStart() != "ignore") {
			registries.at(identifier)->toUnnamed()->add(shared_from_this(), json.at(1));
		}
	}

	RealmID Game::newRealmID() const {
		// TODO: a less stupid way of doing this.
		RealmID max = 1;
		for (const auto &[id, realm]: realms) {
			max = std::max(max, id);
		}
		return max + 1;
	}

	double Game::getTotalSeconds() const {
		return time;
	}

	double Game::getHour() const {
		const auto base = time / 10. + hourOffset;
		return int64_t(base) % 24 + fractional(base);
	}

	double Game::getMinute() const {
		return 60. * fractional(getHour());
	}

	double Game::getDivisor() const {
		return 3. - 2. * std::sin(getHour() * 3.14159265358979323846 / 24.);
	}

	std::optional<TileID> Game::getFluidTileID(FluidID fluid_id) {
		if (auto iter = fluidCache.find(fluid_id); iter != fluidCache.end()) {
			return iter->second;
		}

		if (auto fluid = registry<FluidRegistry>().maybe(static_cast<size_t>(fluid_id))) {
			if (auto tileset = registry<TilesetRegistry>().maybe(fluid->tilesetName)) {
				if (auto fluid_tileid = tileset->maybe(fluid->tilename)) {
					fluidCache.emplace(fluid_id, *fluid_tileid);
					return fluid_tileid;
				}
			}
		}

		return std::nullopt;
	}

	std::shared_ptr<Fluid> Game::getFluid(FluidID fluid_id) const {
		return fluidRegistry->maybe(static_cast<size_t>(fluid_id));
	}

	std::shared_ptr<Fluid> Game::getFluid(const Identifier &identifier) const {
		return fluidRegistry->maybe(identifier);
	}

	GamePtr Game::create(Side side, const GameArgument &argument) {
		GamePtr out;

		if (side == Side::Client) {
			out = GamePtr(new ClientGame(std::get<std::shared_ptr<Window>>(argument)));
		} else {
			const auto [server_ptr, pool_size] = std::get<std::pair<std::shared_ptr<Server>, size_t>>(argument);
			ServerGamePtr server_game = std::make_shared<ServerGame>(server_ptr, pool_size);
			server_game->init();
			out = server_game;
		}

		out->initialSetup();
		return out;
	}

	GamePtr Game::fromJSON(Side side, const boost::json::value &json, const GameArgument &argument) {
		const auto &object = json.as_object();
		auto out = create(side, argument);
		out->initialSetup();
		{
			auto lock = out->realms.uniqueLock();
			for (const auto &[string, realm_json]: boost::json::value_to<std::unordered_map<std::string, boost::json::value>>(json.at("realms"))) {
				out->realms.emplace(parseUlong(string), Realm::fromJSON(out, realm_json));
			}
		}

		if (auto *value = object.if_contains("hourOffset")) {
			out->hourOffset = getDouble(*value);
		} else {
			out->hourOffset = 0;
		}

		if (auto *value = object.if_contains("debugMode")) {
			out->debugMode = value->as_bool();
		} else {
			out->debugMode = false;
		}

		if (auto *value = object.if_contains("cavesGenerated")) {
			out->cavesGenerated = getNumber<size_t>(*value);
		} else {
			out->cavesGenerated = 0;
		}

		return out;
	}

	ClientGame & Game::toClient() {
		return dynamic_cast<ClientGame &>(*this);
	}

	const ClientGame & Game::toClient() const {
		return dynamic_cast<const ClientGame &>(*this);
	}

	std::shared_ptr<ClientGame> Game::toClientPointer() {
		assert(getSide() == Side::Client);
		return std::static_pointer_cast<ClientGame>(shared_from_this());
	}

	ServerGame & Game::toServer() {
		return dynamic_cast<ServerGame &>(*this);
	}

	const ServerGame & Game::toServer() const {
		return dynamic_cast<const ServerGame &>(*this);
	}

	std::shared_ptr<ServerGame> Game::toServerPointer() {
		assert(getSide() == Side::Server);
		return std::static_pointer_cast<ServerGame>(shared_from_this());
	}

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const Game &game) {
		auto &object = json.emplace_object();

		object["debugMode"] = game.debugMode;
		auto &realms = object["realms"].emplace_object();
		game.iterateRealms([&](const RealmPtr &realm) {
			realm->toJSON(realms[std::to_string(realm->id)], true);
		});
		object["hourOffset"] = game.getHour();
		if (0 < game.cavesGenerated) {
			object["cavesGenerated"] = game.cavesGenerated;
		}
	}

#ifndef __MINGW32__
	template <>
	std::shared_ptr<Agent> Game::getAgent<Agent>(GlobalID gid) {
		auto shared_lock = allAgents.sharedLock();
		if (auto iter = allAgents.find(gid); iter != allAgents.end()) {
			if (auto agent = iter->second.lock()) {
				return agent;
			}
			// This should *probably* not result in a data race in practice...
			shared_lock.unlock();
			auto unique_lock = allAgents.uniqueLock();
			allAgents.erase(gid);
		}

		return nullptr;
	}
#endif

	void Game::associateWithRealm(const VillagePtr &village, RealmID realm_id) {
		RealmPtr realm = getRealm(realm_id);
		realm->villages.insert(village);
	}

	bool Game::canLog(int level) const {
		return level <= Logger::level;
	}
}
