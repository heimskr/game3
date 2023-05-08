#pragma once

#include <optional>

#include "data/Identifier.h"
#include "entity/Entity.h"
#include "ui/Modifiers.h"

namespace Game3 {
	class Player: public Entity {
		public:
			static Identifier ID() { return {"base", "entity/player"}; }
			constexpr static HitPoints MAX_HEALTH = 64;

			MoneyCount money;
			float tooldown = 0.f;

			std::unordered_set<Identifier> stationTypes {{}};

			float speed = 10.f;
			bool movingUp = false;
			bool movingRight = false;
			bool movingDown = false;
			bool movingLeft = false;
			/** When moving with shift held, the player will interact with each spot moved to. */
			bool continuousInteraction = false;
			bool ticked = false;

			std::optional<Place> lastContinousInteraction;
			Modifiers continuousInteractionModifiers;

			static std::shared_ptr<Player> fromJSON(Game &, const nlohmann::json &);

			HitPoints maxHealth() const override { return MAX_HEALTH; }
			void toJSON(nlohmann::json &) const override;
			void absorbJSON(Game &, const nlohmann::json &) override;
			bool isPlayer() const override { return true; }
			void tick(Game &, float delta) override;
			void remove() override {}
			bool interactOn();
			void interactNextTo(Modifiers);
			using Entity::teleport;
			void teleport(const Position &, const std::shared_ptr<Realm> &) override;
			void addMoney(MoneyCount);
			float getSpeed() const override { return speed; }
			bool setTooldown(float multiplier);
			inline bool hasTooldown() const { return 0.f < tooldown; }
			void showText(const Glib::ustring &text, const Glib::ustring &name);
			Glib::ustring getName() override { return "Player"; }
			void give(const ItemStack &, Slot start = -1);
			Place getPlace() override;
			bool isMoving() const;

			friend class Entity;

		protected:
			Player();
			void interact(const Position &);
	};

	void to_json(nlohmann::json &, const Player &);

	using PlayerPtr = std::shared_ptr<Player>;
}
