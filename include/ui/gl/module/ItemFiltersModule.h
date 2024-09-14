#pragma once

#include "pipes/ItemFilter.h"
#include "types/DirectedPlace.h"
#include "types/Types.h"
#include "ui/gl/module/Module.h"

#include <any>
#include <memory>

namespace Game3 {
	class Box;
	class Button;
	class Checkbox;
	class Grid;
	class Icon;
	class ItemFilter;
	class ItemSlot;
	class Label;
	class Pipe;
	class TextInput;

	class ItemFiltersModule: public Module {
		public:
			ItemFiltersModule(UIContext &, const std::shared_ptr<ClientGame> &, const std::any &);
			ItemFiltersModule(UIContext &, const std::shared_ptr<ClientGame> &, const DirectedPlace &);

			static Identifier ID() { return {"base", "module/item_filters"}; }

			Identifier getID() const final { return ID(); }
			void init() final;

			using Module::render;
			void render(const RendererContext &, float x, float y, float width, float height) final;

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

		private:
			std::weak_ptr<ClientGame> weakGame;
			DirectedPlace place;
			std::shared_ptr<Pipe> pipe;
			std::shared_ptr<ItemFilter> filter;
			std::shared_ptr<Box> vbox;
			std::shared_ptr<Box> topHBox;
			std::shared_ptr<Box> secondHBox;
			std::shared_ptr<ItemSlot> dropSlot;
			std::shared_ptr<Checkbox> whitelistCheckbox;
			std::shared_ptr<Checkbox> strictCheckbox;
			std::shared_ptr<Grid> grid;

			void copy();
			void paste();

			/** Doesn't update the checkbox. */
			void setWhitelist(bool);

			/** Doesn't update the checkbox. */
			void setStrict(bool);

			void upload(ItemFilterPtr = {});
			bool setFilter();
			bool saveFilter();
			bool setPipe();
			void onDrop(const WidgetPtr &source);
			void populate(ItemFilterPtr filter_to_use = {});

			void addHBox(const Identifier &, const ItemFilter::Config &);
			std::shared_ptr<Icon> makeImage(ItemStack &);
			std::shared_ptr<Label> makeLabel(const ItemStack &);
			std::shared_ptr<Button> makeComparator(const Identifier &, const ItemFilter::Config &);
			std::shared_ptr<TextInput> makeThreshold(const Identifier &, const ItemFilter::Config &); // TODO: implement SpinButton
			std::shared_ptr<Button> makeButton(const ItemStackPtr &);
	};
}
