#pragma once

#include "entity/Entity.h"

namespace Game3 {
	class Merchant: public virtual Entity {
		public:
			static Identifier ID() { return {"base", "entity/merchant"}; }
			static std::shared_ptr<Merchant> create(Game &, EntityType = ID());
			static std::shared_ptr<Merchant> fromJSON(Game &, const nlohmann::json &);

			MoneyCount money = 0;
			double greed = .1;

			void toJSON(nlohmann::json &) const override;
			void absorbJSON(Game &, const nlohmann::json &) override;
			bool onInteractNextTo(const std::shared_ptr<Player> &) override;
			Glib::ustring getName() override { return "Blacksmith"; }
			void encode(Buffer &) override;
			void decode(Buffer &) override;

			friend class Entity;

		protected:
			Merchant() = delete;
			Merchant(EntityType = ID());

			void interact(const Position &);

		private:
			float accumulatedTime = 0.f;
	};

	void to_json(nlohmann::json &, const Merchant &);
}
