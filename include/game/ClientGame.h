#pragma once

#include "game/Game.h"
#include "graphics/Rectangle.h"
#include "threading/Atomic.h"
#include "ui/Modifiers.h"
#include "ui/Sound.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <thread>

namespace Game3 {
	class Window;
	class HasEnergy;
	class HasFluids;
	class HasInventory;
	class LocalClient;
	class Packet;
	class UIContext;

	class ClientGame: public Game {
		public:
			bool stoppedByError = false;
			std::function<void()> errorCallback;
			SoundEngine sounds;
			bool suppressDisconnectionMessage = false;

			ClientGame(const std::shared_ptr<Window> &);

			~ClientGame() override;

			double getFrequency() const override;
			void addEntityFactories() override;
			void click(int button, int n, double pos_x, double pos_y, Modifiers);
			void dragStart(double x, double y, Modifiers);
			void dragUpdate(double x, double y, Modifiers);
			void dragEnd(double x, double y, Modifiers);
			Rectangle getVisibleRealmBounds() const;
			/** Translates coordinates relative to the top left corner of the canvas to realm coordinates. */
			Position translateCanvasCoordinates(double x, double y, double *x_offset_out = nullptr, double *y_offset_out = nullptr) const;
			void activateContext();
			void setText(const Glib::ustring &text);
			void runCommand(const std::string &);
			bool tick() final;
			void queuePacket(std::shared_ptr<Packet>);
			void chunkReceived(ChunkPosition);
			void interactOn(Modifiers, Hand = Hand::None);
			void interactNextTo(Modifiers, Hand = Hand::None);
			void putInLimbo(EntityPtr, RealmID, const Position &);
			void requestFromLimbo(RealmID);
			/** Returns whether a sound was found with the given ID. */
			bool playSound(const Identifier &, float pitch = 1.f);
			UIContext & getUIContext() const;

			void moduleMessageBuffer(const Identifier &module_id, const std::shared_ptr<Agent> &source, const std::string &name, Buffer &&data);

			template <typename... Args>
			void moduleMessage(const Identifier &module_id, const std::shared_ptr<Agent> &source, const std::string &name, Args &&...args) {
				moduleMessageBuffer(module_id, source, name, Buffer{std::forward<Args>(args)...});
			}

			auto signalPlayerInventoryUpdate() const { return signal_player_inventory_update; }
			auto signalPlayerMoneyUpdate()     const { return signal_player_money_update;     }
			auto signalOtherInventoryUpdate()  const { return signal_other_inventory_update;  }
			auto signalFluidUpdate()           const { return signal_fluid_update;            }
			auto signalEnergyUpdate()          const { return signal_energy_update;           }
			auto signalVillageUpdate()         const { return signal_village_update;          }

			Side getSide() const final { return Side::Client; }

			inline ClientPlayerPtr getPlayer() const { return player; }
			inline std::shared_ptr<LocalClient> getClient() const { return client; }
			inline RealmPtr getActiveRealm() const { return activeRealm; }

			void setPlayer(ClientPlayerPtr);
			void setClient(std::shared_ptr<LocalClient>);
			void setActiveRealm(RealmPtr);

			/** Returns whether the thread could be started. The thread can't be started if the thread is already running. */
			bool startThread();
			void stopThread();

			std::shared_ptr<ClientGame> getSelf() { return std::static_pointer_cast<ClientGame>(shared_from_this()); }
			std::shared_ptr<const ClientGame> getSelf() const { return std::static_pointer_cast<const ClientGame>(shared_from_this()); }

			std::shared_ptr<Window> getWindow() const;

		private:
			std::weak_ptr<Window> weakWindow;
			ClientPlayerPtr player;
			std::shared_ptr<LocalClient> client;
			RealmPtr activeRealm;
			sigc::signal<void(const PlayerPtr &)> signal_player_inventory_update;
			sigc::signal<void(const PlayerPtr &)> signal_player_money_update;
			sigc::signal<void(const std::shared_ptr<Agent> &, InventoryID)> signal_other_inventory_update;
			sigc::signal<void(const std::shared_ptr<HasFluids> &)> signal_fluid_update;
			sigc::signal<void(const std::shared_ptr<HasEnergy> &)> signal_energy_update;
			sigc::signal<void(const std::shared_ptr<Village> &)> signal_village_update;
			std::atomic_bool active{false};
			std::thread tickThread;
			std::optional<Position> lastDragPosition;
			float lastGarbageCollection = 0;

			Lockable<std::set<ChunkPosition>> missingChunks;
			MTQueue<std::shared_ptr<Packet>> packetQueue;
			/** Temporarily stores shared pointers to entities that have moved to a realm we're unaware of to prevent destruction. */
			Lockable<std::unordered_map<RealmID, std::unordered_map<EntityPtr, Position>>> entityLimbo;

			void garbageCollect();
	};

	using ClientGamePtr = std::shared_ptr<ClientGame>;
}
