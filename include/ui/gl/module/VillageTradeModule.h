#pragma once

#include "ui/gl/module/Module.h"
#include "ui/gl/widget/Box.h"

namespace Game3 {
	class Button;
	class Item;
	class ItemSlot;
	class Label;
	class TextInput;
	class Village;

	class VillageTradeRow: public Box {
		public:
			VillageTradeRow(UIContext &, float scale, const std::shared_ptr<ClientGame> &, VillageID, const Item &item, double amount_);

			void init() final;
			void setAmount(double);
			void updateLabel();
			void updateTooltips(ItemCount);

		private:
			std::weak_ptr<ClientGame> weakGame;
			VillageID villageID{};
			Identifier resource;
			std::shared_ptr<ItemSlot> itemSlot;
			// double basePrice{};
			double amount{};
			std::shared_ptr<Label> quantityLabel;
			std::shared_ptr<TextInput> transferAmount;
			std::shared_ptr<Button> buyButton;

			ItemCount getCount() const;
			void buy(const std::shared_ptr<ClientGame> &, ItemCount);
			void sell(const std::shared_ptr<ClientGame> &, ItemCount);
	};

	class VillageTradeModule: public Module {
		public:
			static Identifier ID() { return {"base", "module/village_trade"}; }

			VillageTradeModule(UIContext &, const std::shared_ptr<ClientGame> &, const std::any &);

			Identifier getID() const final { return ID(); }using Module::render;
			void init() final;
			void reset() final;
			void update() final;

			void render(const RendererContext &, float x, float y, float width, float height) final;
			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

			std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final;
			// bool handleShiftClick(std::shared_ptr<Inventory>, Slot) final;

		private:
			std::map<Identifier, std::shared_ptr<VillageTradeRow>> rows;
			bool sellRowShown = false;

			std::shared_ptr<Village> village;
			std::shared_ptr<Box> vbox;
			std::shared_ptr<Label> villageName;
			std::shared_ptr<Label> laborLabel;
			std::shared_ptr<Box> sellRow;
			std::shared_ptr<ItemSlot> sellSlot;
			std::shared_ptr<TextInput> sellCount;
			std::shared_ptr<Button> sellButton;
			std::shared_ptr<Box> sellLabelBox;
			std::shared_ptr<Label> totalPriceLabel;
			std::shared_ptr<Label> unitPriceLabel;

			bool setSellStack(ItemStackPtr);
			void updateSell(const ItemStackPtr &);
			void sell();
			void showSell();
			void hideSell();
			void populate();
	};
}
