#pragma once

#include "tileentity/TileEntity.h"

namespace Game3 {
	class EntityBuilding: public TileEntity {
		public:
			static Identifier ID() { return {"base", "te/entity_building"}; }
			GlobalID targetEntity{};

			std::string getName() const override { return "Entity Building"; }

			void toJSON(nlohmann::json &) const override;
			void absorbJSON(const std::shared_ptr<Game> &, const nlohmann::json &) override;
			bool onInteractOn(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			void teleport(const std::shared_ptr<Entity> &);

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

		protected:
			EntityBuilding() = default;
			EntityBuilding(Identifier tilename, Position position_, GlobalID target_entity);

		friend class TileEntity;
	};
}
