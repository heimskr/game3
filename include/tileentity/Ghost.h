#pragma once

#include <functional>

#include "Tileset.h"
#include "item/Item.h"
#include "tileentity/TileEntity.h"

namespace Game3 {
	class Game;
	struct GhostDetails;

	// enum class GhostType {Invalid, Normal, WoodenWall, Tower, Custom};

	class GhostFunction: public NamedRegisterable {
		private:
			std::function<bool(const Identifier &, const Place &)> function;

		public:
			GhostFunction(Identifier, decltype(function));
			bool operator()(const Identifier &tilename, const Place &) const;
	};

	struct GhostDetails: NamedRegisterable {
		/** Note: the player field of the Place will be null! */
		using CustomFn = std::function<void(const Place &)>;

		// GhostType type = GhostType::Invalid;
		Identifier type;
		Identifier tilesetName;
		bool useMarchingSquares = false;
		Index columnsPerRow = 16;
		Index rowOffset     = 0;
		Index columnOffset  = 0;
		Index layer = 2;
		CustomFn customFn;
		Identifier customTilename;

		GhostDetails(Identifier identifier_ = {}):
			NamedRegisterable(std::move(identifier_)) {}

		GhostDetails(Identifier identifier_, Identifier type_, Identifier tileset_name, bool use_marching_squares, Index columns_per_row, Index row_offset, Index column_offset, Index layer_):
			NamedRegisterable(std::move(identifier_)),
			type(std::move(type_)),
			tilesetName(std::move(tileset_name)),
			useMarchingSquares(use_marching_squares),
			columnsPerRow(columns_per_row),
			rowOffset(row_offset),
			columnOffset(column_offset),
			layer(layer_) {}

		GhostDetails(Identifier identifier_, Identifier tileset_name, CustomFn custom_fn, Identifier custom_tile_name):
			NamedRegisterable(std::move(identifier_)),
			type("base:ghost/custom"),
			tilesetName(std::move(tileset_name)),
			customFn(std::move(custom_fn)),
			customTilename(std::move(custom_tile_name)) {}

		static GhostDetails & get(const Game &, const ItemStack &);
	};

	void initGhosts(Game &);

	class Ghost: public TileEntity {
		public:
			static Identifier ID() { return {"base", "te/ghost"}; }
			GhostDetails details;
			ItemStack material;
			TileID marched = 0;

			Ghost(const Ghost &) = delete;
			Ghost(Ghost &&) = default;
			~Ghost() override = default;

			Ghost & operator=(const Ghost &) = delete;
			Ghost & operator=(Ghost &&) = default;

			void toJSON(nlohmann::json &) const override;
			void absorbJSON(Game &, const nlohmann::json &) override;
			void onSpawn() override;
			void onNeighborUpdated(Index row_offset, Index column_offset) override;
			bool onInteractNextTo(const std::shared_ptr<Player> &) override;
			void render(SpriteRenderer &) override;
			/** This method doesn't remove the tile entity or decrement the realm's ghost count by itself. */
			void confirm();

			friend class TileEntity;

		protected:
			Ghost() = default;
			Ghost(const Place &place, ItemStack material_);

		private:
			void march();
	};
}
