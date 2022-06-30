#pragma once

#include "entity/Worker.h"

namespace Game3 {
	/** Lives in a town and produces tools during the day. */
	class Blacksmith: public Worker {
		public:
			constexpr static float BUYING_TIME = 5.f;
			constexpr static float CRAFTING_TIME = 5.f;
			/** The number of iron bars, gold bars and diamonds that the blacksmith will try to maintain. */
			constexpr static ItemCount RESOURCE_TARGET = 64;

			static std::shared_ptr<Blacksmith> create(EntityID, RealmID overworld_realm, RealmID house_realm, const Position &house_position, const std::shared_ptr<Building> &keep_);
			static std::shared_ptr<Blacksmith> fromJSON(const nlohmann::json &);

			void toJSON(nlohmann::json &) const override;
			void absorbJSON(const nlohmann::json &) override;
			bool onInteractNextTo(const std::shared_ptr<Player> &) override;
			void tick(Game &, float delta) override;

			friend class Entity;

		protected:
			float actionTime = 0.f;

			Blacksmith(EntityID);
			Blacksmith(EntityID, RealmID overworld_realm, RealmID house_realm, const Position &house_position, const std::shared_ptr<Building> &keep_);

			void interact(const Position &);

		private:
			ItemCount coalNeeded = 0;
			ItemCount ironOreNeeded = 0;
			ItemCount goldOreNeeded = 0;
			ItemCount diamondOreNeeded = 0;

			void wakeUp();
			void buyResources();
			void goToForge();
			void craftTools();
			void goToCounter();
			void startSelling();
	};
}
