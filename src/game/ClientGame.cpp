#include "Log.h"
#include "Options.h"
#include "command/local/LocalCommandFactory.h"
#include "entity/ClientPlayer.h"
#include "entity/EntityFactory.h"
#include "error/Warning.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "game/SimulationOptions.h"
#include "graphics/Tileset.h"
#include "net/DisconnectedError.h"
#include "net/LocalClient.h"
#include "packet/CommandPacket.h"
#include "packet/ChunkRequestPacket.h"
#include "packet/ClickPacket.h"
#include "packet/DragPacket.h"
#include "packet/EntityRequestPacket.h"
#include "packet/InteractPacket.h"
#include "packet/RegisterPlayerPacket.h"
#include "packet/TeleportSelfPacket.h"
#include "threading/ThreadContext.h"
#include "ui/gl/Constants.h"
#include "ui/Window.h"
#include "util/Util.h"

namespace Game3 {
	namespace {
		constexpr float GARBAGE_COLLECTION_TIME = 60;
	}

	ClientGame::ClientGame(const std::shared_ptr<Window> &window):
		Game(),
		weakWindow(window),
		tickThreadLaunchWaiter(1) {}

	ClientGame::~ClientGame() {
		INFO(3, "\e[31m~ClientGame\e[39m({})", reinterpret_cast<void *>(this));
	}

	double ClientGame::getFrequency() const {
		return getWindow()->settings.tickFrequency;
	}

	void ClientGame::addEntityFactories() {
		Game::addEntityFactories();
		add(EntityFactory::create<ClientPlayer>());
	}

	void ClientGame::click(int button, int, double pos_x, double pos_y, Modifiers modifiers) {
		RealmPtr realm = activeRealm;

		if (!realm)
			return;

		double fractional_x = 0.;
		double fractional_y = 0.;

		const Position translated = translateCanvasCoordinates(pos_x, pos_y, &fractional_x, &fractional_y);

		auto client = getClient();

		if (button == LEFT_BUTTON) {
			client->send(make<ClickPacket>(translated, fractional_x, fractional_y, modifiers));
		} else if (button == RIGHT_BUTTON && getPlayer() && !realm->rightClick(translated, pos_x, pos_y) && debugMode && client && client->isConnected()) {
			client->send(make<TeleportSelfPacket>(realm->id, translated));
		}
	}

	void ClientGame::dragStart(double x, double y, Modifiers modifiers) {
		Position position = translateCanvasCoordinates(x, y);
		lastDragPosition = position;
		getClient()->send(make<DragPacket>(DragPacket::Action::Start, position, modifiers));
	}

	void ClientGame::dragUpdate(double x, double y, Modifiers modifiers) {
		Position position = translateCanvasCoordinates(x, y);
		if (lastDragPosition && *lastDragPosition != position) {
			lastDragPosition = position;
			getClient()->send(make<DragPacket>(DragPacket::Action::Update, position, modifiers));
		}
	}

	void ClientGame::dragEnd(double x, double y, Modifiers modifiers) {
		if (lastDragPosition) {
			Position position = translateCanvasCoordinates(x, y);
			lastDragPosition.reset();
			getClient()->send(make<DragPacket>(DragPacket::Action::End, position, modifiers));
		}
	}

	Rectangle ClientGame::getVisibleRealmBounds() const {
		auto window = getWindow();
		const auto [top,     left] = translateCanvasCoordinates(0, 0);
		const auto [bottom, right] = translateCanvasCoordinates(window->getWidth(), window->getHeight());
		return {
			static_cast<int>(left),
			static_cast<int>(top),
			static_cast<int>(right - left + 1),
			static_cast<int>(bottom - top + 1),
		};
	}

	Position ClientGame::translateCanvasCoordinates(double x, double y, double *x_offset_out, double *y_offset_out) const {
		RealmPtr realm = activeRealm;

		if (!realm)
			return {};

		std::shared_ptr<Window> window = getWindow();

		const int width = window->getWidth();
		const int height = window->getHeight();

		const auto scale = window->scale;
		const auto tile_size = realm->getTileset().getTileSize();
		constexpr auto map_length = CHUNK_SIZE * REALM_DIAMETER;
		x -= width  / 2. - (map_length * tile_size / 4.) * scale + window->center.first  * window->magic * scale;
		y -= height / 2. - (map_length * tile_size / 4.) * scale + window->center.second * window->magic * scale;
		const double sub_x = x < 0.? 1. : 0.;
		const double sub_y = y < 0.? 1. : 0.;
		x /= tile_size * scale / 2.;
		y /= tile_size * scale / 2.;

		double intpart{};

		// The math here is bizarre. Probably tied to the getQuadrant silliness.
		if (x_offset_out)
			*x_offset_out = std::abs(1 - sub_x - std::abs(std::modf(x, &intpart)));

		if (y_offset_out)
			*y_offset_out = std::abs(1 - sub_y - std::abs(std::modf(y, &intpart)));

		return {static_cast<Index>(y - sub_y), static_cast<Index>(x - sub_x)};
	}

	void ClientGame::activateContext() {
		getWindow()->activateContext();
	}

	void ClientGame::setText(const UString &text) {
		auto window = getWindow();
		window->showOmniDialog();
		window->openModule("base:module/text", std::any(text.raw()));
	}

	void ClientGame::runCommand(const std::string &command) {
		auto pieces = split<std::string>(command, " ", false);

		if (pieces.empty())
			throw CommandError("No command entered");

		if (auto factory = registry<LocalCommandFactoryRegistry>().maybe(pieces.front())) {
			auto command = (*factory)();
			command->pieces = std::move(pieces);
			(*command)(*getClient());
		} else {
			getClient()->send(make<CommandPacket>(threadContext.rng(), command));
		}
	}

	bool ClientGame::tick() {
		if (!Game::tick())
			return false;

		lastGarbageCollection += delta;
		if (lastGarbageCollection >= GARBAGE_COLLECTION_TIME) {
			lastGarbageCollection = 0;
			garbageCollect();
		}

#ifndef USE_TLS
		getClient()->read();
#endif

		for (const auto &packet: packetQueue.steal()) {
			try {
				packet->handle(getSelf());
			} catch (const Warning &warning) {
				getWindow()->error(warning.what());
			} catch (const std::exception &err) {
				auto &packet_ref = *packet;
				ERROR("Couldn't handle packet of type {} ({}): {}", DEMANGLE(packet_ref), packet->getID(), err.what());
				throw;
			}
		}

		if (!getPlayer())
			return true;

		for (const auto &[realm_id, realm]: realms)
			realm->tick(delta);

		if (auto realm = getPlayer()->getRealm()) {
			auto missing_chunks_lock = missingChunks.sharedLock();
			if (missingChunks.empty()) {
				auto new_missing_chunks = realm->getMissingChunks();
				missing_chunks_lock.unlock();
				missingChunks = std::move(new_missing_chunks);
				missing_chunks_lock.lock();
				if (!missingChunks.empty())
					getClient()->send(make<ChunkRequestPacket>(*realm, missingChunks, true));
			}
		} else {
			WARN("No realm");
		}

		return true;
	}

	void ClientGame::queuePacket(std::shared_ptr<Packet> packet) {
		packetQueue.push(std::move(packet));
	}

	void ClientGame::chunkReceived(ChunkPosition chunk_position) {
		auto lock = missingChunks.uniqueLock();
		missingChunks.erase(chunk_position);
	}

	void ClientGame::interactOn(Modifiers modifiers, Hand hand) {
		if (ClientPlayerPtr player = getPlayer()) {
			auto client = getClient();
			assert(client);
			client->send(make<InteractPacket>(true, hand, modifiers, std::nullopt, player->direction));
		}
	}

	void ClientGame::interactNextTo(Modifiers modifiers, Hand hand) {
		if (ClientPlayerPtr player = getPlayer()) {
			auto client = getClient();
			assert(client);
			client->send(make<InteractPacket>(false, hand, modifiers, std::nullopt, player->direction));
		}
	}

	void ClientGame::putInLimbo(EntityPtr entity, RealmID next_realm_id, const Position &next_position) {
		auto lock = entityLimbo.uniqueLock();
		entity->getRealm()->queueRemoval(entity);
		entity->inLimboFor = next_realm_id;
		entityLimbo[next_realm_id][std::move(entity)] = next_position;
	}

	void ClientGame::requestFromLimbo(RealmID realm_id) {
		auto lock = entityLimbo.uniqueLock();
		if (auto iter = entityLimbo.find(realm_id); iter != entityLimbo.end()) {
			std::vector<EntityRequest> requests;
			const std::unordered_map<EntityPtr, Position> &entity_map = iter->second;
			requests.reserve(entity_map.size());
			for (const auto &[entity, position]: entity_map)
				requests.emplace_back(entity->getGID(), 0);
			// TODO: is this safe/reasonable?
			// Safe in the sense that we aren't erasing something we shouldn't erase.
			entityLimbo.erase(iter);
			lock.unlock();
			getPlayer()->send(make<EntityRequestPacket>(realm_id, std::move(requests)));
		}
	}

	bool ClientGame::playSound(const Identifier &identifier, float pitch) {
		if (const std::filesystem::path *path = getSound(identifier)) {
			sounds.play(*path, pitch);
			return true;
		}

		return false;
	}

	UIContext & ClientGame::getUIContext() const {
		return getWindow()->uiContext;
	}

	void ClientGame::handleChat(const PlayerPtr &player, const UString &message) {
		signalChatReceived(player, message);
	}

	void ClientGame::moduleMessageBuffer(const Identifier &module_id, const std::shared_ptr<Agent> &source, const std::string &name, Buffer &&data) {
		getWindow()->moduleMessageBuffer(module_id, source, name, std::move(data));
	}

	void ClientGame::setPlayer(ClientPlayerPtr new_player) {
		player = std::move(new_player);
	}

	void ClientGame::setClient(std::shared_ptr<LocalClient> new_client) {
		client = std::move(new_client);
	}

	void ClientGame::setActiveRealm(RealmPtr new_realm) {
		activeRealm = std::move(new_realm);
	}

	bool ClientGame::startThread() {
		if (active.exchange(true))
			return false;

		stoppedByError = false;

		tickThreadLaunchWaiter.reset(1, true);

		tickThread = std::thread([this] {
			while (active) {
				try {
					if (!tick()) {
						active = false;
						break;
					}
				} catch (const DisconnectedError &) {
					active = false;
					stoppedByError = true;
					break;
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(1000 / getWindow()->settings.tickFrequency));
			}

			if (stoppedByError && errorCallback)
				errorCallback();
		});

		--tickThreadLaunchWaiter;

		return true;
	}

	void ClientGame::stopThread() {
		if (!tickThread.joinable()) {
			tickThreadLaunchWaiter.wait();
		}

		if (tickThread.joinable()) {
			active = false;
			tickThread.join();
		} else {
			WARN("Trying to stop an unjoinable ClientGame");
		}
	}

	std::shared_ptr<Window> ClientGame::getWindow() const {
		auto window = weakWindow.lock();
		assert(window);
		return window;
	}

	bool ClientGame::isConnectedLocally() const {
		if (!cachedIsConnectedLocally) {
			cachedIsConnectedLocally = getWindow()->isConnectedLocally();
		}

		return *cachedIsConnectedLocally;
	}

	void ClientGame::garbageCollect() {
		sounds.cleanup();
	}
}
