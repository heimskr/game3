#pragma once

#include "container/WeakSet.h"
#include "game/HasPlace.h"
#include "net/Buffer.h"
#include "threading/Lockable.h"
#include "types/ChunkPosition.h"
#include "ui/Modifiers.h"

#include <gtkmm.h>

namespace Game3 {
	class Game;
	class ItemStack;
	class Player;

	struct AgentMeta {
		UpdateCounter updateCounter = 0;
		AgentMeta() = default;
		AgentMeta(UpdateCounter counter): updateCounter(counter) {}
	};

	class Agent: public HasPlace, public std::enable_shared_from_this<Agent> {
		public:
			enum class Type {Entity, TileEntity};

			GlobalID globalID = generateGID();
			bool initialized = false;

			virtual ~Agent() = default;

			std::vector<ChunkPosition> getVisibleChunks() const;

			virtual Side getSide() const = 0;
			virtual Type getAgentType() const = 0;
			virtual std::string getName() const = 0;

			/** Handles when the player interacts with the tile they're on and that tile contains this agent. Returns whether anything interesting happened. */
			virtual bool onInteractOn(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &used_item, Hand);
			/** Handles when the player interacts with the tile in front of them and that tile contains this agent. Returns whether anything interesting happened. */
			virtual bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &used_item, Hand);

			virtual bool populateMenu(const PlayerPtr &, bool overlap, const std::string &id, Glib::RefPtr<Gio::Menu>, Glib::RefPtr<Gio::SimpleActionGroup>) { (void) overlap; (void) id; return false; }

			virtual void handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data);
			virtual void sendMessage(const std::shared_ptr<Agent> &destination, const std::string &name, std::any &data);

			template <typename... Args>
			void sendMessage(const std::shared_ptr<Agent> &destination, const std::string &name, Args &&...args) {
				std::any data{Buffer{std::forward<Args>(args)...}};
				sendMessage(destination, name, data);
			}

			virtual GlobalID getGID() const { return globalID; }
			virtual void setGID(GlobalID new_gid) { globalID = new_gid; }
			inline bool hasGID() const { return globalID != static_cast<GlobalID>(-1); }
			static bool validateGID(GlobalID);

			inline auto getUpdateCounter() {
				auto lock = agentMeta.sharedLock();
				return agentMeta.updateCounter;
			}

			inline auto increaseUpdateCounter() {
				auto lock = agentMeta.uniqueLock();
				return ++agentMeta.updateCounter;
			}

			inline void setUpdateCounter(UpdateCounter new_counter) {
				auto lock = agentMeta.uniqueLock();
				agentMeta.updateCounter = new_counter;
			}

			bool hasBeenSentTo(const std::shared_ptr<Player> &);
			void onSend(const std::shared_ptr<Player> &);

			/** Called by Inventory::notifyOwner even if inventory updates are suppressed. */
			virtual void inventoryUpdated(InventoryID) {}

			static GlobalID generateGID();

		private:
			Lockable<AgentMeta> agentMeta;
			Lockable<WeakSet<Player>> sentTo;
	};

	using AgentPtr = std::shared_ptr<Agent>;
}
