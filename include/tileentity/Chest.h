#pragma once

#include <optional>

#include "Texture.h"
#include "game/Inventory.h"
#include "tileentity/TileEntity.h"

namespace Game3 {
	class Chest: public TileEntity {
		public:
			static Texture DEFAULT_TEXTURE;

			Texture texture;
			std::optional<Inventory> inventory;

			Chest(const Chest &) = delete;
			Chest(Chest &&) = default;
			~Chest() override = default;

			Chest & operator=(const Chest &) = delete;
			Chest & operator=(Chest &&) = default;

			TileEntityID getID() const override { return TileEntity::CHEST; }

			void toJSON(nlohmann::json &) const override;
			void onInteractNextTo(const std::shared_ptr<Player> &) override;
			void absorbJSON(const nlohmann::json &) override;
			void render(SpriteRenderer &) override;
			void setInventory(Slot slot_count);

		protected:
			Chest() = default;
			Chest(TileID id_, const Position &position_, const Texture &texture_ = DEFAULT_TEXTURE);

			friend class TileEntity;
	};
}
