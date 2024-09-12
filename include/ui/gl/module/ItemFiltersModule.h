#pragma once

#include "types/DirectedPlace.h"
#include "types/Types.h"
#include "ui/gl/module/Module.h"

#include <any>
#include <memory>

namespace Game3 {
	class ItemSlot;
	class Checkbox;
	class Grid;

	class ItemFiltersModule: public Module {
		public:
			ItemFiltersModule(const std::shared_ptr<ClientGame> &, const std::any &);
			ItemFiltersModule(const std::shared_ptr<ClientGame> &, const DirectedPlace &);

			static Identifier ID() { return {"base", "module/item_filters"}; }

			Identifier getID() const final { return ID(); }
			void init(UIContext &) final;

			using Module::render;
			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

		private:
			std::weak_ptr<ClientGame> weakGame;
			DirectedPlace place;
			std::shared_ptr<ItemSlot> dropSlot;
			std::shared_ptr<Checkbox> whitelistCheckbox;
			std::shared_ptr<Checkbox> strictCheckbox;
			std::shared_ptr<Grid> grid;

			void copy();
			void paste();
	};
}
