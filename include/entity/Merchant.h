#pragma once

#include "entity/Entity.h"

namespace Game3 {
	class Merchant: public virtual Entity {
		public:
			static Identifier ID() { return {"base", "entity/merchant"}; }
			static std::shared_ptr<Merchant> create(const std::shared_ptr<Game> &, EntityType = ID());
			static std::shared_ptr<Merchant> fromJSON(const std::shared_ptr<Game> &, const boost::json::value &);

			double greed = .1;

			void toJSON(boost::json::value &) const override;
			void absorbJSON(const std::shared_ptr<Game> &, const boost::json::value &) override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			std::string getName() const override { return "Blacksmith"; }
			void encode(Buffer &) override;
			void decode(BasicBuffer &) override;

		friend class Entity;

		protected:
			Merchant() = delete;
			Merchant(EntityType = ID());

			void interact(const Position &);

		private:
			float accumulatedTime = 0.f;
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &, const Merchant &);
}
