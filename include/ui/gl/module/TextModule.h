#pragma once

// #include "graphics/Rectangle.h"
#include "types/Types.h"
#include "ui/gl/module/Module.h"

#include <glibmm/ustring.h> // TODO: remove

#include <any>
#include <memory>

namespace Game3 {
	class ClientGame;

	class TextModule: public Module {
		public:
			TextModule(std::shared_ptr<ClientGame>, const std::any &);
			TextModule(std::shared_ptr<ClientGame>, std::string);

			static Identifier ID() { return {"base", "module/text"}; }

			Identifier getID() const final { return ID(); }

			void render(UIContext &, RendererContext &, float x, float y, float width, float height) final;

		private:
			Glib::ustring text;
	};
}
