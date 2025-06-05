#pragma once

#include "client/ClientSettings.h"
#include "game/Game.h"
#include "graphics/Omniatlas.h"
#include "math/Rectangle.h"
#include "threading/Atomic.h"
#include "threading/Waiter.h"
#include "types/UString.h"
#include "ui/Modifiers.h"
#include "ui/Sound.h"

#include <sigc++/sigc++.h>

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <thread>

namespace Game3 {
	class HasEnergy;
	class HasFluids;
	class HasInventory;
	class LocalClient;
	class Packet;
	class UIContext;
	class Window;

	class ClientGame: public Game {
		public:
			bool stoppedByError = false;
			bool suppressDisconnectionMessage = false;
			std::function<void()> errorCallback;
			SoundEngine sounds;
			std::optional<Omniatlas> omniatlas;

			sigc::signal<void(const PlayerPtr &)> signalPlayerInventoryUpdate;
			sigc::signal<void(const PlayerPtr &)> signalPlayerMoneyUpdate;
			sigc::signal<void(const std::shared_ptr<Agent> &, InventoryID)> signalOtherInventoryUpdate;
			sigc::signal<void(const std::shared_ptr<HasFluids> &)> signalFluidUpdate;
			sigc::signal<void(const std::shared_ptr<HasEnergy> &)> signalEnergyUpdate;
			sigc::signal<void(const std::shared_ptr<Village> &)> signalVillageUpdate;
			sigc::signal<void(const PlayerPtr &, const UString &)> signalChatReceived;

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
			std::pair<double, double> untranslateCanvasCoordinates(Position) const;
			void activateContext();
			void setText(const UString &text);
			void runCommand(const std::string &);
			bool tick() final;
			void queuePacket(std::shared_ptr<Packet>);
			void send(const std::shared_ptr<Packet> &);
			void chunkReceived(ChunkPosition);
			void interactOn(Modifiers, Hand = Hand::None);
			void interactNextTo(Modifiers, Hand = Hand::None);
			void putInLimbo(EntityPtr, RealmID, const Position &);
			void requestFromLimbo(RealmID);
			/** Returns whether a sound was found with the given ID. */
			bool playSound(const Identifier &, float pitch = 1, float volume = 1);
			UIContext & getUIContext() const;
			void handleChat(const PlayerPtr &, const UString &message);

			void moduleMessageBuffer(const Identifier &module_id, const std::shared_ptr<Agent> &source, const std::string &name, Buffer &&data);

			template <typename... Args>
			void moduleMessage(const Identifier &module_id, const std::shared_ptr<Agent> &source, const std::string &name, Args &&...args) {
				moduleMessageBuffer(module_id, source, name, Buffer{std::forward<Args>(args)...});
			}

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
			Lockable<ClientSettings> & getSettings() const;

			bool isConnectedLocally() const;

			void initialSetup(const std::filesystem::path &dir) final;

			template <typename... Args>
			void runCommand(std::format_string<Args...> format, Args &&...args) {
				runCommand(std::format(format, std::forward<Args>(args)...));
			}

		private:
			std::weak_ptr<Window> weakWindow;
			ClientPlayerPtr player;
			std::shared_ptr<LocalClient> client;
			RealmPtr activeRealm;
			std::atomic_bool active{false};
			std::thread tickThread;
			Waiter tickThreadLaunchWaiter;
			std::optional<Position> lastDragPosition;
			float lastGarbageCollection = 0;
			mutable std::optional<bool> cachedIsConnectedLocally;

			Lockable<std::set<ChunkPosition>> missingChunks;
			MTQueue<std::shared_ptr<Packet>> packetQueue;
			/** Temporarily stores shared pointers to entities that have moved to a realm we're unaware of to prevent destruction. */
			Lockable<std::unordered_map<RealmID, std::unordered_map<EntityPtr, Position>>> entityLimbo;

			void garbageCollect();
	};

	using ClientGamePtr = std::shared_ptr<ClientGame>;
}
