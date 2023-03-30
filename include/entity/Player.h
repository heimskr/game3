#pragma once

#include <optional>

#include "entity/Entity.h"

namespace Game3 {
	class Player: public Entity {
		public:
			MoneyCount money;
			float tooldown = 0.f;

			std::unordered_set<CraftingStationType> stationTypes {CraftingStationType::None};

			float speed = 10.f;
			bool movingUp = false;
			bool movingRight = false;
			bool movingDown = false;
			bool movingLeft = false;
			/** When moving with shift held, the player will interact with each spot moved to. */
			bool continuousInteraction = false;
			bool ticked = false;

			std::optional<Place> lastContinousInteraction;

			static std::shared_ptr<Player> fromJSON(const nlohmann::json &);

			void toJSON(nlohmann::json &) const override;
			void absorbJSON(const nlohmann::json &) override;
			bool isPlayer() const override { return true; }
			void tick(Game &, float delta) override;
			void remove() override {}
			void interactOn();
			void interactNextTo();
			using Entity::teleport;
			void teleport(const Position &, const std::shared_ptr<Realm> &) override;
			void addMoney(MoneyCount);
			float getSpeed() const override { return speed; }
			bool setTooldown(float multiplier);
			inline bool hasTooldown() const { return 0.f < tooldown; }
			void showText(const Glib::ustring &text, const Glib::ustring &name);
			Glib::ustring getName() override { return "Player"; }
			void give(const ItemStack &, Slot start = -1);
			Place getPlace();
			bool isMoving() const;

			friend class Entity;

		protected:
			Player(EntityID);
			void interact(const Position &);
	};

	void to_json(nlohmann::json &, const Player &);

	using PlayerPtr = std::shared_ptr<Player>;
}
