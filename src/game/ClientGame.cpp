#include "Log.h"
#include "threading/ThreadContext.h"
#include "Tileset.h"
#include "command/local/LocalCommandFactory.h"
#include "entity/ClientPlayer.h"
#include "entity/EntityFactory.h"
#include "error/Warning.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "game/SimulationOptions.h"
#include "net/DisconnectedError.h"
#include "net/LocalClient.h"
#include "packet/CommandPacket.h"
#include "packet/ChunkRequestPacket.h"
#include "packet/ClickPacket.h"
#include "packet/EntityRequestPacket.h"
#include "packet/InteractPacket.h"
#include "packet/RegisterPlayerPacket.h"
#include "packet/TeleportSelfPacket.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/tab/TextTab.h"
#include "util/Util.h"

namespace Game3 {
	void ClientGame::addEntityFactories() {
		Game::addEntityFactories();
		add(EntityFactory::create<ClientPlayer>());
	}

	void ClientGame::click(int button, int, double pos_x, double pos_y, Modifiers modifiers) {
		RealmPtr realm = activeRealm.copyBase();

		if (!realm)
			return;

		double fractional_x = 0.;
		double fractional_y = 0.;

		const Position translated = translateCanvasCoordinates(pos_x, pos_y, &fractional_x, &fractional_y);

		if (button == 1)
			client->send(ClickPacket(translated, fractional_x, fractional_y, modifiers));
		else if (button == 3 && player && !realm->rightClick(translated, pos_x, pos_y) && debugMode && client && client->isConnected())
			client->send(TeleportSelfPacket(realm->id, translated));
	}

	Gdk::Rectangle ClientGame::getVisibleRealmBounds() const {
		const auto [top,     left] = translateCanvasCoordinates(0., 0.);
		const auto [bottom, right] = translateCanvasCoordinates(canvas.width(), canvas.height());
		return {
			static_cast<int>(left),
			static_cast<int>(top),
			static_cast<int>(right - left + 1),
			static_cast<int>(bottom - top + 1),
		};
	}

	MainWindow & ClientGame::getWindow() {
		return canvas.window;
	}

	Position ClientGame::translateCanvasCoordinates(double x, double y, double *x_offset_out, double *y_offset_out) const {
		RealmPtr realm = activeRealm.copyBase();

		if (!realm)
			return {};

		const auto scale = canvas.scale;
		const auto tile_size = realm->getTileset().getTileSize();
		constexpr auto map_length = CHUNK_SIZE * REALM_DIAMETER;
		x -= canvas.width() / 2.f - (map_length * tile_size / 4.f) * scale + canvas.center.x() * canvas.magic * scale;
		y -= canvas.height() / 2.f - (map_length * tile_size / 4.f) * scale + canvas.center.y() * canvas.magic * scale;
		const double sub_x = x < 0.? 1. : 0.;
		const double sub_y = y < 0.? 1. : 0.;
		x /= tile_size * scale / 2.f;
		y /= tile_size * scale / 2.f;

		double intpart = 0.;

		// The math here is bizarre. Probably tied to the getQuadrant silliness.
		if (x_offset_out)
			*x_offset_out = std::abs(1 - sub_x - std::abs(std::modf(x, &intpart)));

		if (y_offset_out)
			*y_offset_out = std::abs(1 - sub_y - std::abs(std::modf(y, &intpart)));

		return {static_cast<Index>(y - sub_y), static_cast<Index>(x - sub_x)};
	}

	void ClientGame::activateContext() {
		canvas.window.activateContext();
	}

	void ClientGame::setText(const Glib::ustring &text, const Glib::ustring &name, bool focus, bool ephemeral) {
		if (canvas.window.textTab) {
			auto &tab = *canvas.window.textTab;
			tab.text = text;
			tab.name = name;
			tab.ephemeral = ephemeral;
			if (focus)
				tab.show();
			tab.reset(toClientPointer());
		}
	}

	const Glib::ustring & ClientGame::getText() const {
		if (canvas.window.textTab)
			return canvas.window.textTab->text;
		throw std::runtime_error("Can't get text: TextTab is null");
	}

	void ClientGame::runCommand(const std::string &command) {
		auto pieces = split<std::string>(command, " ", false);

		if (pieces.empty())
			throw CommandError("No command entered");

		if (auto factory = registry<LocalCommandFactoryRegistry>().maybe(pieces.front())) {
			auto command = (*factory)();
			command->pieces = std::move(pieces);
			(*command)(*client);
		} else
			client->send(CommandPacket(threadContext.rng(), command));
	}

	bool ClientGame::tick() {
		if (!Game::tick())
			return false;

		client->read();

		for (const auto &packet: packetQueue.steal()) {
			try {
				packet->handle(*this);
			} catch (const Warning &warning) {
				canvas.window.error(warning.what());
			} catch (const std::exception &err) {
				auto &packet_ref = *packet;
				ERROR("Couldn't handle packet of type " << typeid(packet_ref).name() << " (" << packet->getID() << "): " << err.what());
				throw;
			}
		}

		if (!player)
			return true;

		for (const auto &[realm_id, realm]: realms)
			realm->tick(delta);

		if (auto realm = player->getRealm()) {
			auto missing_chunks_lock = missingChunks.sharedLock();
			if (missingChunks.empty()) {
				auto new_missing_chunks = realm->getMissingChunks();
				missing_chunks_lock.unlock();
				missingChunks = std::move(new_missing_chunks);
				missing_chunks_lock.lock();
				if (!missingChunks.empty())
					client->send(ChunkRequestPacket(*realm, missingChunks, true));
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

	void ClientGame::interactOn(Modifiers modifiers) {
		assert(client);
		client->send(InteractPacket(true, modifiers));
	}

	void ClientGame::interactNextTo(Modifiers modifiers) {
		assert(client);
		client->send(InteractPacket(false, modifiers));
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
			player->send(EntityRequestPacket(realm_id, std::move(requests)));
		}
	}

	void ClientGame::moduleMessageBuffer(const Identifier &module_id, const std::shared_ptr<Agent> &source, const std::string &name, Buffer &&data) {
		getWindow().moduleMessageBuffer(module_id, source, name, std::move(data));
	}

	bool ClientGame::startThread() {
		if (active.exchange(true))
			return false;

		stoppedByError = false;

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

				std::this_thread::sleep_for(std::chrono::milliseconds(TICK_PERIOD));
			}

			if (stoppedByError && errorCallback)
				errorCallback();
		});

		return true;
	}

	void ClientGame::stopThread() {
		if (tickThread.joinable()) {
			active = false;
			tickThread.join();
		} else
			WARN("Trying to stop an unjoinable ClientGame");
	}
}
