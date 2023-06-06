#pragma once

#include "Texture.h"
#include "game/HasInventory.h"
#include "game/Inventory.h"
#include "tileentity/TileEntity.h"

namespace Game3 {
	class Chest: public HasInventory, public TileEntity {
		public:
			static Identifier ID() { return {"base", "te/chest"}; }
			static constexpr const char * DEFAULT_TEXTURE_PATH = "resources/rpg/chests.png";

			std::string name;
			std::shared_ptr<Texture> texture;

			Chest(const Chest &) = delete;
			Chest(Chest &&) = default;
			~Chest() override = default;

			Chest & operator=(const Chest &) = delete;
			Chest & operator=(Chest &&) = default;

			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &) override;
			void absorbJSON(Game &, const nlohmann::json &) override;
			void render(SpriteRenderer &) override;
			void setInventory(Slot slot_count);
			void inventoryUpdated() override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

		protected:
			Chest() = default;
			Chest(Identifier tile_id, const Position &, std::string name_, std::shared_ptr<Texture> = cacheTexture(DEFAULT_TEXTURE_PATH));

			std::shared_ptr<Agent> getSharedAgent() override { return std::dynamic_pointer_cast<Chest>(shared_from_this()); }

			friend class TileEntity;
	};
}
