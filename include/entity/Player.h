#pragma once

#include <optional>

#include "data/Identifier.h"
#include "entity/Entity.h"
#include "entity/LivingEntity.h"
#include "ui/Modifiers.h"

namespace Game3 {
	class ClientPlayer;
	class Packet;
	class ServerPlayer;
	class TileEntity;

	class Player: public LivingEntity {
		public:
			static Identifier ID() { return {"base", "entity/player"}; }
			constexpr static HitPoints MAX_HEALTH = 64;

			/** Not encoded. */
			Lockable<std::string> username;
			/** Definitely not encoded. */
			Token token = -1;
			std::string displayName;
			float tooldown = 0;
			Lockable<std::unordered_set<Identifier>> stationTypes{{}};
			float movementSpeed = 10;
			float timeSinceAttack = 0;
			std::atomic_bool movingUp = false;
			std::atomic_bool movingRight = false;
			std::atomic_bool movingDown = false;
			std::atomic_bool movingLeft = false;
			/** When moving with shift held, the player will interact with each spot moved to. */
			bool continuousInteraction = false;
			bool ticked = false;
			Atomic<RealmID> spawnRealmID;
			Lockable<Position> spawnPosition;

			std::optional<Place> lastContinuousInteraction;
			Modifiers continuousInteractionModifiers;

			~Player() override = 0;
			void destroy() override;

			HitPoints getMaxHealth() const override;
			void toJSON(nlohmann::json &) const override;
			void absorbJSON(Game &, const nlohmann::json &) override;
			bool isPlayer() const override { return true; }
			void tick(Game &, float delta) override;
			void remove() override {}
			bool interactOn(Modifiers, ItemStack *used_item = nullptr, Hand = Hand::None);
			void interactNextTo(Modifiers, ItemStack *used_item = nullptr, Hand = Hand::None);
			using Entity::teleport;
			void teleport(const Position &, const std::shared_ptr<Realm> &, MovementContext) override;
			void addMoney(MoneyCount);
			float getMovementSpeed() const override { return movementSpeed; }
			bool setTooldown(float multiplier);
			inline bool hasTooldown() const { return 0.f < tooldown; }
			void showText(const Glib::ustring &text, const Glib::ustring &name);
			std::string getName() const override { return "Player"; }
			void give(const ItemStack &, Slot start = -1);
			Place getPlace() override;
			bool isMoving() const;
			bool isMoving(Direction) const;
			void setupRealm(const Game &);
			void encode(Buffer &) override;
			void decode(Buffer &) override;
			void startMoving(Direction);
			void stopMoving();
			void stopMoving(Direction);
			void movedToNewChunk(const std::optional<ChunkPosition> &) override;
			bool send(const Packet &);
			void addStationType(Identifier);
			void removeStationType(const Identifier &);
			std::shared_ptr<Player> getShared();
			std::shared_ptr<ClientPlayer> toClient();
			std::shared_ptr<ServerPlayer> toServer();
			void addKnownRealm(RealmID);
			bool knowsRealm(RealmID) const;
			void notifyOfRealm(Realm &);
			int getZIndex() const override { return 1; }
			float getAttackPeriod() const;
			bool canAttack() const;

			friend class Entity;

		protected:
			Lockable<std::unordered_set<RealmID>> knownRealms;

			Player();

			/** Resets client-side things like movingUp and the continuous interaction fields that aren't transferred over the network. */
			void resetEphemeral();
	};

	void to_json(nlohmann::json &, const Player &);

	using PlayerPtr = std::shared_ptr<Player>;
}
