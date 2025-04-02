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
			constexpr static float HARVESTING_TIME = 5.f;
			constexpr static float SELLING_TIME = 5.f;

			std::optional<Position> chosenResource;
			float harvestingTime = 0.f;

			static std::shared_ptr<Miner> create(const std::shared_ptr<Game> &);
			static std::shared_ptr<Miner> create(const std::shared_ptr<Game> &, RealmID overworld_realm, RealmID house_realm, const Position &house_position, const std::shared_ptr<Building> &keep_);
			static std::shared_ptr<Miner> fromJSON(const std::shared_ptr<Game> &, const boost::json::value &);

			void toJSON(boost::json::value &) const override;
			void absorbJSON(const std::shared_ptr<Game> &, const boost::json::value &) override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			void tick(const TickArgs &) override;
			std::string getName() const override { return "Miner"; }
			void encode(Buffer &) override;
			void decode(Buffer &) override;

		friend class Entity;

		protected:
			float sellTime = 0.f;

			Miner();
			Miner(RealmID overworld_realm, RealmID house_realm, const Position &house_position, const std::shared_ptr<Building> &keep_);

			void interact(const Position &);

		private:
			void wakeUp();
			void goToResource();
			void startHarvesting();
			void harvest(float delta);
			void sellInventory();
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &, const Miner &);
}
