#include "graphics/Tileset.h"
#include "game/ClientGame.h"
#include "game/Fluids.h"
#include "game/Game.h"
#include "game/InteractionSet.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "ui/gl/module/AutocrafterModule.h"
#include "ui/gl/module/EnergyModule.h"
#include "ui/gl/module/FluidsModule.h"
#include "ui/gl/module/GeneInfoModule.h"
#include "ui/gl/module/InventoryModule.h"
#include "ui/gl/module/MicroscopeModule.h"
#include "ui/gl/module/ModuleFactory.h"
#include "ui/gl/module/MultiModule.h"
#include "ui/gl/module/MutatorModule.h"
#include "ui/gl/module/TextModule.h"
#include "ui/module/ChemicalReactorModule.h"
#include "ui/module/CombinerModule.h"
#include "ui/module/ComputerModule.h"
#include "ui/module/GTKInventoryModule.h"
#include "ui/module/GTKEnergyLevelModule.h"
#include "ui/module/GTKItemFilterModule.h"
#include "ui/module/GTKModuleFactory.h"
#include "ui/module/VillageTradeModule.h"
#include "algorithm/AStar.h"
#include "util/FS.h"
#include "util/Timer.h"
#include "util/Util.h"

#include <nlohmann/json.hpp>

namespace Game3 {
	Game::Game():
		debugMode(false) {}

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
		add(GTKModuleFactory::create<GTKInventoryModule>());
		add(GTKModuleFactory::create<ChemicalReactorModule>());
		add(GTKModuleFactory::create<GTKEnergyLevelModule>());
		add(GTKModuleFactory::create<GTKItemFilterModule>());
		add(GTKModuleFactory::create<CombinerModule>());
		add(GTKModuleFactory::create<VillageTradeModule>());
		add(ModuleFactory::create<AutocrafterModule>());
		add(ModuleFactory::create<EnergyModule>());
		add(ModuleFactory::create<FluidsModule>());
		add(ModuleFactory::create<InventoryModule>());
		add(ModuleFactory::create<MicroscopeModule<0>>());
		add(ModuleFactory::create<MicroscopeModule<1, Substance::Energy>>());
		add(ModuleFactory::create<MicroscopeModule<1, Substance::Energy, Substance::Fluid>>());
		add(ModuleFactory::create<MicroscopeModule<2, Substance::Energy>>());
		add(ModuleFactory::create<MutatorModule>());
		add(ModuleFactory::create<MultiModule<Substance::Item, Substance::Energy, Substance::Fluid>>());
		add(ModuleFactory::create<MultiModule<Substance::Item, Substance::Fluid>>());
		add(ModuleFactory::create<MultiModule<Substance::Item, Substance::Energy>>());
		add(ModuleFactory::create<MultiModule<Substance::Energy, Substance::Fluid>>());
		add(ModuleFactory::create<TextModule>());
#ifdef GAME3_ENABLE_SCRIPTING
		add(GTKModuleFactory::create<ComputerModule>());
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
	}

	void Game::initEntities() {
		for (const auto &[realm_id, realm]: realms)
			realm->initEntities();
	}

	void Game::initInteractionSets() {
		interactionSets.clear();
		auto standard = std::make_shared<StandardInteractions>();
		for (const auto &type: registry<RealmTypeRegistry>().items)
			interactionSets.emplace(type, standard);
	}

	void Game::add(std::shared_ptr<Item> item) {
		registry<ItemRegistry>().add(item->identifier, item);
		for (const auto &attribute: item->attributes)
			itemsByAttribute[attribute].insert(item);
	}

	void Game::add(GTKModuleFactory &&factory) {
		auto shared = std::make_shared<GTKModuleFactory>(std::move(factory));
		registry<GTKModuleFactoryRegistry>().add(shared->identifier, shared);
	}

	void Game::add(ModuleFactory &&factory) {
		auto shared = std::make_shared<ModuleFactory>(std::move(factory));
		registry<ModuleFactoryRegistry>().add(shared->identifier, shared);
	}

	void Game::addRecipe(const nlohmann::json &json) {
		const Identifier identifier = json.at(0);
		if (identifier.getPathStart() != "ignore")
			registries.at(identifier)->toUnnamed()->add(shared_from_this(), json.at(1));
	}

	RealmID Game::newRealmID() const {
		// TODO: a less stupid way of doing this.
		RealmID max = 1;
		for (const auto &[id, realm]: realms)
			max = std::max(max, id);
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
		return 3. - 2. * std::sin(getHour() * M_PI / 24.);
	}

	std::optional<TileID> Game::getFluidTileID(FluidID fluid_id) {
		if (auto iter = fluidCache.find(fluid_id); iter != fluidCache.end())
			return iter->second;

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
		return registry<FluidRegistry>().maybe(static_cast<size_t>(fluid_id));
	}

	std::shared_ptr<Fluid> Game::getFluid(const Identifier &identifier) const {
		return registry<FluidRegistry>().maybe(identifier);
	}

	GamePtr Game::create(Side side, const GameArgument &argument) {
		GamePtr out;

		if (side == Side::Client) {
			out = GamePtr(new ClientGame(*std::get<Canvas *>(argument)));
		} else {
			const auto [server_ptr, pool_size] = std::get<std::pair<std::shared_ptr<Server>, size_t>>(argument);
			ServerGamePtr server_game = std::make_shared<ServerGame>(server_ptr, pool_size);
			server_game->init();
			out = server_game;
		}

		out->initialSetup();
		return out;
	}

	GamePtr Game::fromJSON(Side side, const nlohmann::json &json, const GameArgument &argument) {
		auto out = create(side, argument);
		out->initialSetup();
		{
			auto lock = out->realms.uniqueLock();
			for (const auto &[string, realm_json]: json.at("realms").get<std::unordered_map<std::string, nlohmann::json>>())
				out->realms.emplace(parseUlong(string), Realm::fromJSON(out, realm_json));
		}
		out->hourOffset = json.contains("hourOffset")? json.at("hourOffset").get<float>() : 0.f;
		out->debugMode = json.contains("debugMode")? json.at("debugMode").get<bool>() : false;
		out->cavesGenerated = json.contains("cavesGenerated")? json.at("cavesGenerated").get<decltype(Game::cavesGenerated)>() : 0;
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

	void to_json(nlohmann::json &json, const Game &game) {
		json["debugMode"] = game.debugMode;
		json["realms"] = std::unordered_map<std::string, nlohmann::json>();
		game.iterateRealms([&](const RealmPtr &realm) {
			realm->toJSON(json["realms"][std::to_string(realm->id)], true);
		});
		json["hourOffset"] = game.getHour();
		if (0 < game.cavesGenerated)
			json["cavesGenerated"] = game.cavesGenerated;
	}

	template <>
	std::shared_ptr<Agent> Game::getAgent<Agent>(GlobalID gid) {
		auto shared_lock = allAgents.sharedLock();
		if (auto iter = allAgents.find(gid); iter != allAgents.end()) {
			if (auto agent = iter->second.lock())
				return agent;
			// This should *probably* not result in a data race in practice...
			shared_lock.unlock();
			auto unique_lock = allAgents.uniqueLock();
			allAgents.erase(gid);
		}

		return nullptr;
	}

	void Game::associateWithRealm(const VillagePtr &village, RealmID realm_id) {
		RealmPtr realm = getRealm(realm_id);
		realm->villages.insert(village);
	}

	bool Game::canLog(int level) const {
		return level <= Logger::level;
	}
}
