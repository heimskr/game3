#pragma once

#include "data/Identifier.h"
#include "entity/Entity.h"
#include "entity/LivingTitledEntity.h"
#include "game/CraftingManager.h"
#include "types/UString.h"
#include "ui/Modifiers.h"

#include <optional>

namespace Game3 {
	class ClientPlayer;
	class Packet;
	class ServerPlayer;
	class TileEntity;

	class Player: public LivingTitledEntity {
		public:
			static Identifier ID() { return {"base", "entity/player"}; }
			constexpr static HitPoints MAX_HEALTH = 64;

			/** Not encoded. */
			Lockable<std::string> username;
			std::string displayName;
			Lockable<std::unordered_set<Identifier>> stationTypes{{}};
			/** Definitely not encoded. */
			Token token = -1;
			float tooldown = 0;
			float timeSinceAttack = 0;
			std::atomic_bool movingUp = false;
			std::atomic_bool movingRight = false;
			std::atomic_bool movingDown = false;
			std::atomic_bool movingLeft = false;
			/** When moving with shift held, the player will interact with each spot moved to. */
			bool continuousInteraction = false;
			bool ticked = false;
			/** Whether the player is clicking and holding on the world. */
			bool firing = false;
			Atomic<RealmID> spawnRealmID;
			Lockable<Position> spawnPosition;
			std::optional<Place> lastContinuousInteraction;
			Modifiers continuousInteractionModifiers;
			CraftingManager craftingManager{*this};

			~Player() override = 0;
			void destroy() override;

			HitPoints getMaxHealth() const override;
			void toJSON(boost::json::value &) const override;
			void absorbJSON(const std::shared_ptr<Game> &, const boost::json::value &) override;
			bool isPlayer() const override { return true; }
			void tick(const TickArgs &) override;
			void remove() override {}
			bool interactOn(Modifiers, const ItemStackPtr &used_item = nullptr, Hand = Hand::None);
			void interactNextTo(Modifiers, const ItemStackPtr &used_item = nullptr, Hand = Hand::None);
			using Entity::teleport;
			void teleport(const Position &, const std::shared_ptr<Realm> &, MovementContext) override;
			virtual void addMoney(MoneyCount) = 0;
			/** Returns whether the player had enough money. If false, no change was made. */
			virtual bool removeMoney(MoneyCount) = 0;
			bool setTooldown(float multiplier, const ItemStackPtr &used_item);
			inline bool hasTooldown() const { return 0.f < tooldown; }
			virtual void showText(const UString &text, const UString &name) = 0;
			std::string getName() const override { return "Player"; }
			void give(const ItemStackPtr &, Slot start = -1);
			Place getPlace() override;
			bool isMoving() const;
			bool isMoving(Direction) const;
			void setupRealm(const Game &);
			void encode(Buffer &) override;
			void decode(Buffer &) override;
			void startMoving(Direction);
			void stopMoving();
			void stopMoving(Direction);
			bool send(const PacketPtr &);
			void addStationType(Identifier);
			void removeStationType(const Identifier &);
			bool hasStationType(const Identifier &) const;
			std::shared_ptr<Player> getShared();
			std::shared_ptr<ClientPlayer> toClient();
			std::shared_ptr<ServerPlayer> toServer();
			void addKnownRealm(RealmID);
			bool knowsRealm(RealmID) const;
			void notifyOfRealm(Realm &);
			int getZIndex() const override { return 1; }
			float getAttackPeriod() const;
			bool canAttack() const;
			/** Returns whether the item was added (i.e., not already in the set). */
			bool addKnownItem(const ItemStackPtr &);
			/** Returns whether the item was added (i.e., not already in the set). */
			bool addKnownItem(const Identifier &);
			void setKnownItems(std::set<Identifier>);
			bool hasKnownItem(const Identifier &) const;
			virtual void setFiring(bool);
			Direction getSecondaryDirection() const override;
			UString getDisplayName() override;

			inline std::string getUsername() const { return username.copyBase(); }
			inline const auto & getKnownItems() const { return knownItems; }

		protected:
			Lockable<std::unordered_set<RealmID>> knownRealms;
			Lockable<std::set<Identifier>> knownItems;

			Player();

			/** Resets client-side things like movingUp and the continuous interaction fields that aren't transferred over the network. */
			void resetEphemeral();

		friend class Entity;
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &, const Player &);

	using PlayerPtr = std::shared_ptr<Player>;
}
