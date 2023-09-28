#pragma once

#include "item/Item.h"

namespace Game3 {
	struct AutotileSet;

	struct Furniture: Item {
		using Item::Item;
		bool use(Slot, ItemStack &, const Place &, Modifiers, std::pair<float, float>) override;
		virtual Layer getLayer() const = 0;
		virtual bool apply(const Place &) = 0;

		static std::shared_ptr<Furniture> create(ItemID id_, std::string name_, MoneyCount base_price, Layer, Identifier tilename);
		static std::shared_ptr<Furniture> create(ItemID id_, std::string name_, MoneyCount base_price, Layer, Identifier start, Identifier autotile);
	};

	struct SimpleFurniture: Furniture {
		Identifier tilename;
		Layer layer;

		SimpleFurniture(ItemID, std::string name_, MoneyCount base_price, Layer, Identifier tilename_);

		Layer getLayer() const override { return layer; }
		bool apply(const Place &) override;
	};

	class MarchableFurniture: public Furniture {
		public:
			MarchableFurniture(ItemID, std::string name_, MoneyCount base_price, Layer, Identifier start_, Identifier autotile_);

			Layer getLayer() const override { return layer; }
			bool apply(const Place &) override;

		private:
			Identifier start;
			Identifier autotile;
			Layer layer;

			TileID march(const Place &place);
	};
}
