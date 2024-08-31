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
			TextModule(std::shared_ptr<ClientGame>, const std::any &);
			TextModule(std::shared_ptr<ClientGame>, std::string);

			static Identifier ID() { return {"base", "module/text"}; }

			Identifier getID() const final { return ID(); }

			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;
			std::pair<float, float> calculateSize(const RendererContext &, float available_width, float available_height) final;

			void setText(UIContext &, UString);

		private:
			std::weak_ptr<ClientGame> weakGame;
			UString text;
			std::optional<UString> wrapped;
			float lastTextHeight = -1;

			float getTextScale() const;
			float getPadding() const;
			float getWrapWidth(float width) const;
			void tryWrap(const TextRenderer &);
			void tryWrap();
			ClientGame & getGame() const;
	};
}
