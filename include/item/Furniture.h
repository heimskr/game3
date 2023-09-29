#pragma once

#include "item/Item.h"

#include <functional>

namespace Game3 {
	struct AutotileSet;
	class TileEntity;

	struct Furniture: Item {
		using Item::Item;
		bool use(Slot, ItemStack &, const Place &, Modifiers, std::pair<float, float>) override;
		virtual Layer getLayer() const = 0;
		virtual bool apply(const Place &) = 0;

		static std::shared_ptr<Furniture> createSimple(ItemID id_, std::string name_, MoneyCount base_price, Layer, Identifier tilename);
		static std::shared_ptr<Furniture> createMarchable(ItemID id_, std::string name_, MoneyCount base_price, Layer, Identifier start, Identifier autotile);
		static std::shared_ptr<Furniture> createCustom(ItemID id_, std::string name_, MoneyCount base_price, std::function<bool(const Place &)> placer);
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

	class CustomFurniture: public Furniture {
		public:
			CustomFurniture(ItemID, std::string name_, MoneyCount base_price, std::function<bool(const Place &)> placer_, Layer = Layer::Objects);
			bool apply(const Place &) override;
			Layer getLayer() const override { return layer; }

		private:
			std::function<bool(const Place &)> placer;
			Layer layer;
	};
}
