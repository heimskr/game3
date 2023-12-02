#pragma once

#include "Layer.h"
#include "types/Types.h"
#include "data/Identifier.h"
#include "item/Item.h"
#include "ui/Modifiers.h"

#include <functional>
#include <string>
#include <utility>

namespace Game3 {
	class TileEntity;
	struct AutotileSet;
	struct Place;

	class Furniture: public Item {
		public:
			using Item::Item;

			bool use(Slot, ItemStack &, const Place &, Modifiers, std::pair<float, float>, Hand) override;
			bool drag(Slot, ItemStack &, const Place &, Modifiers) override;

			virtual bool preCheck(const Place &) const;
			virtual Layer getLayer() const = 0;
			virtual bool apply(const Place &) = 0;

			static std::shared_ptr<Furniture> createSimple(ItemID id, std::string name, MoneyCount base_price, Layer, Identifier tilename);
			static std::shared_ptr<Furniture> createMarchable(ItemID id, std::string name, MoneyCount base_price, Layer, Identifier start, Identifier autotile);
			static std::shared_ptr<Furniture> createCustom(ItemID id, std::string name, MoneyCount base_price, std::function<bool(const Place &)> placer);
			static std::shared_ptr<Furniture> createTileEntity(ItemID id, std::string name, MoneyCount base_price, std::function<bool(const Place &)> placer);
			static std::shared_ptr<Furniture> createStation(ItemID id, std::string name, MoneyCount base_price, Identifier tilename, Identifier station_name);
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

		protected:
			std::function<bool(const Place &)> placer;
			Layer layer;
	};

	class TileEntityFurniture: public CustomFurniture {
		public:
			TileEntityFurniture(ItemID, std::string name_, MoneyCount base_price, std::function<bool(const Place &)> placer_);
			bool preCheck(const Place &) const override;
	};

	class StationFurniture: public Furniture {
		public:
			Identifier tilename;
			Identifier stationType;

			StationFurniture(ItemID, std::string name_, MoneyCount base_price, Identifier tilename_, Identifier station_type);
			bool preCheck(const Place &) const override;
			Layer getLayer() const override { return Layer::Invalid; }
			bool apply(const Place &) override;
	};
}
