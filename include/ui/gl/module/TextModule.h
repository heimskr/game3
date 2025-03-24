#pragma once

#include "types/Types.h"
#include "types/UString.h"
#include "ui/gl/module/Module.h"

#include <any>
#include <memory>
#include <optional>

namespace Game3 {
	class ClientGame;
	class TextRenderer;

	class TextModule: public Module {
		public:
			TextModule(UIContext &, float selfScale, std::shared_ptr<ClientGame>, const std::any &);
			TextModule(UIContext &, float selfScale, std::shared_ptr<ClientGame>, std::string);

			static Identifier ID() { return {"base", "module/text"}; }

			Identifier getID() const final { return ID(); }

			using Module::render;
			void render(const RendererContext &, float x, float y, float width, float height) final;

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

			void setText(UIContext &, UString);

		private:
			UString text;
			std::optional<UString> wrapped;
			float lastTextHeight = -1;

			float getTextScale() const;
			float getPadding() const;
			float getWrapWidth(float width) const;
			void tryWrap(const TextRenderer &);
			void tryWrap();
	};
}
