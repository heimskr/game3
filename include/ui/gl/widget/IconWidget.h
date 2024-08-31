#pragma once

#include "ui/gl/widget/Widget.h"
#include "ui/gl/HasFixedHeight.h"

namespace Game3 {
	class IconWidget: public Widget, public HasFixedHeight {
		public:
			IconWidget(float scale);

			using Widget::render;
			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;
			float calculateHeight(const RendererContext &, float available_width, float available_height) final;

			void setIconTexture(TexturePtr);

		protected:
			TexturePtr iconTexture;
	};
}
