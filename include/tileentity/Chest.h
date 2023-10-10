#pragma once

#include "graphics/Texture.h"
#include "game/Inventory.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	class Chest: public InventoriedTileEntity {
		public:
			static Identifier ID() { return {"base", "te/chest"}; }

			std::string name;
			Identifier itemName{"base", "item/chest"};

			Chest(const Chest &) = delete;
			Chest(Chest &&) = default;
			~Chest() override = default;

			Chest & operator=(const Chest &) = delete;
			Chest & operator=(Chest &&) = default;

			std::string getName() const override;

			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers) override;
			void absorbJSON(Game &, const nlohmann::json &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

		protected:
			Chest() = default;
			Chest(Identifier tile_id, const Position &, std::string name_, Identifier item_name = {"base", "item/chest"});
			Chest(const Position &);

			friend class TileEntity;
	};
}
