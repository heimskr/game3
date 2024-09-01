#pragma once

#include "ui/gl/widget/Widget.h"
#include "ui/gl/HasFixedSize.h"

namespace Game3 {
	class IconWidget: public Widget, public HasFixedSize {
		public:
			IconWidget(float scale);

			using Widget::render;
			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

			void setIconTexture(TexturePtr);

		protected:
			TexturePtr iconTexture;
	};
}
