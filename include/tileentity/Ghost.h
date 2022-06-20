#pragma once

#include "Tiles.h"
#include "item/Item.h"
#include "tileentity/TileEntity.h"

namespace Game3 {
	class Texture;

	enum class GhostType {Invalid, Normal, WoodenWall};

	struct GhostDetails {
		GhostType type = GhostType::Invalid;
		Texture *texture = nullptr;
		bool useMarchingSquares = false;
		Index columnsPerRow = 16;
		Index rowOffset     = 0;
		Index columnOffset  = 0;

		GhostDetails() = default;
		GhostDetails(GhostType type_, Index columns_per_row, Index row_offset, Index column_offset):
			type(type_), useMarchingSquares(true), columnsPerRow(columns_per_row), rowOffset(row_offset), columnOffset(column_offset) {}

		static GhostDetails WOODEN_WALL;

		static GhostDetails & get(const ItemStack &);
	};

	class Ghost: public TileEntity {
		public:
			GhostDetails details;
			ItemStack material;
			TileID marched = 0;

			Ghost(const Ghost &) = delete;
			Ghost(Ghost &&) = default;
			~Ghost() override = default;

			Ghost & operator=(const Ghost &) = delete;
			Ghost & operator=(Ghost &&) = default;

			TileEntityID getID() const override { return TileEntity::GHOST; }

			void toJSON(nlohmann::json &) const override;
			void absorbJSON(const nlohmann::json &) override;
			void onSpawn();
			void onNeighborUpdated(Index row_offset, Index column_offset) override;
			bool onInteractNextTo(const std::shared_ptr<Player> &) override;
			void render(SpriteRenderer &) override;
			/** This method doesn't remove the tile entity or decrement the realm's ghost count by itself. */
			void confirm();

			friend class TileEntity;

		protected:
			Ghost() = default;

			Ghost(const Position &position_, const ItemStack &material_):
				TileEntity(Monomap::MISSING, TileEntity::GHOST, position_, true), details(GhostDetails::get(material_)), material(material_) {}

		private:
			void march();
	};
}
