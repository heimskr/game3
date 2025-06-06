#pragma once

#include "ui/gl/widget/Widget.h"
#include "ui/gl/HasFixedSize.h"
#include "ui/gl/HasTooltipText.h"

namespace Game3 {
	class Icon: public Widget, public HasFixedSize, public HasTooltipText {
		public:
			Icon(UIContext &, float scale, const std::filesystem::path &icon_path);
			Icon(UIContext &, float scale, TexturePtr iconTexture);
			Icon(UIContext &, float scale);

			using Widget::render;
			void render(const RendererContext &, float x, float y, float width, float height) final;

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

			void setIconTexture(TexturePtr);

		protected:
			TexturePtr iconTexture;
	};
}
