#pragma once

#include "types/Layer.h"
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

			bool use(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>) override;
			bool drag(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>) override;

			virtual bool preCheck(const Place &) const;
			virtual Layer getLayer() const = 0;
			virtual bool apply(const Place &) = 0;
			virtual Identifier getSoundSet() const;

			static std::shared_ptr<Furniture> createSimple(ItemID id, std::string name, MoneyCount base_price, Layer, Identifier tilename, Identifier sound_set_id = {});
			static std::shared_ptr<Furniture> createMarchable(ItemID id, std::string name, MoneyCount base_price, Layer, Identifier start, Identifier autotile);
			static std::shared_ptr<Furniture> createCustom(ItemID id, std::string name, MoneyCount base_price, std::function<bool(const Place &)> placer);
			static std::shared_ptr<Furniture> createTileEntity(ItemID id, std::string name, MoneyCount base_price, std::function<bool(const Place &)> placer);
			static std::shared_ptr<Furniture> createStation(ItemID id, std::string name, MoneyCount base_price, Identifier tilename, Identifier station_name);
	};

	struct SimpleFurniture: Furniture {
		Identifier tilename;
		Identifier soundSetID;
		Layer layer;

		SimpleFurniture(ItemID, std::string name, MoneyCount basePrice, Layer, Identifier tilename, Identifier soundSetID);

		Layer getLayer() const override { return layer; }
		bool apply(const Place &) override;
		Identifier getSoundSet() const override;
	};

	class MarchableFurniture: public Furniture {
		public:
			MarchableFurniture(ItemID, std::string name, MoneyCount basePrice, Layer, Identifier start, Identifier autotile);

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
			CustomFurniture(ItemID, std::string name, MoneyCount basePrice, std::function<bool(const Place &)> placer, Layer = Layer::Objects);
			bool apply(const Place &) override;
			Layer getLayer() const override { return layer; }

		protected:
			std::function<bool(const Place &)> placer;
			Layer layer;
	};

	class TileEntityFurniture: public CustomFurniture {
		public:
			TileEntityFurniture(ItemID, std::string name, MoneyCount basePrice, std::function<bool(const Place &)> placer);
			bool preCheck(const Place &) const override;
	};

	class StationFurniture: public Furniture {
		public:
			Identifier tilename;
			Identifier stationType;

			StationFurniture(ItemID, std::string name, MoneyCount basePrice, Identifier tilename, Identifier stationType);
			bool preCheck(const Place &) const override;
			Layer getLayer() const override { return Layer::Invalid; }
			bool apply(const Place &) override;
	};
}
