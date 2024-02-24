#pragma once

#include <optional>

#include "types/Types.h"
#include "entity/Worker.h"

namespace Game3 {
	/** Lives in a town and mines ore during the day. */
	class Miner: public Worker {
		public:
			static Identifier ID() { return {"base", "entity/miner"}; }
			constexpr static Index RADIUS = 50;
			constexpr static double HARVESTING_TIME = 5.;
			constexpr static double SELLING_TIME = 5.;

			std::optional<Position> chosenResource;
			double harvestingTime = 0.;

			static std::shared_ptr<Miner> create(Game &);
			static std::shared_ptr<Miner> create(Game &, RealmID overworld_realm, RealmID house_realm, const Position &house_position, const std::shared_ptr<Building> &keep_);
			static std::shared_ptr<Miner> fromJSON(Game &, const nlohmann::json &);

			void toJSON(nlohmann::json &) const override;
			void absorbJSON(Game &, const nlohmann::json &) override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, ItemStack *, Hand) override;
			void tick(const TickArgs &) override;
			std::string getName() const override { return "Miner"; }
			void encode(Buffer &) override;
			void decode(Buffer &) override;

			friend class Entity;

		protected:
			double sellTime = 0.;

			Miner();
			Miner(RealmID overworld_realm, RealmID house_realm, const Position &house_position, const std::shared_ptr<Building> &keep_);

			void interact(const Position &);

		private:
			void wakeUp();
			void goToResource();
			void startHarvesting();
			void harvest(double delta);
			void sellInventory();
	};

	void to_json(nlohmann::json &, const Miner &);
}
