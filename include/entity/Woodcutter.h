#pragma once

#include "entity/Worker.h"

namespace Game3 {
	/** Lives in a town and chops wood during the day. */
	class Woodcutter: public Worker {
		public:
			static Identifier ID() { return {"base", "entity/woodcutter"}; }
			constexpr static Index RADIUS = 50;
			constexpr static float HARVESTING_TIME = 5.f;
			constexpr static float SELLING_TIME = 5.f;

			std::optional<Position> chosenResource;
			float harvestingTime = 0.f;

			static std::shared_ptr<Woodcutter> create(const std::shared_ptr<Game> &);
			static std::shared_ptr<Woodcutter> create(const std::shared_ptr<Game> &, RealmID overworld_realm, RealmID house_realm, Position house_position, std::shared_ptr<Building> keep_);
			static std::shared_ptr<Woodcutter> fromJSON(const std::shared_ptr<Game> &, const nlohmann::json &);

			void toJSON(nlohmann::json &) const override;
			void absorbJSON(const std::shared_ptr<Game> &, const nlohmann::json &) override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			void tick(const TickArgs &) override;
			std::string getName() const override { return "Woodcutter"; }
			void encode(Buffer &) override;
			void decode(Buffer &) override;

		friend class Entity;

		protected:
			float sellTime = 0.f;

			Woodcutter();
			Woodcutter(RealmID overworld_realm, RealmID house_realm, Position house_position, std::shared_ptr<Building> keep_);

			void interact(const Position &);

		private:
			void wakeUp();
			void goToResource();
			void startHarvesting();
			void harvest(float delta);
			void sellInventory();
	};

	void to_json(nlohmann::json &, const Woodcutter &);
}
