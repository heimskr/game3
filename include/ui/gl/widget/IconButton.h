#pragma once

#include "ui/gl/widget/Button.h"

namespace Game3 {
	class IconButton: public Button {
		public:
			using Button::Button;

			void setIconTexture(TexturePtr);

		protected:
			TexturePtr iconTexture;

			void renderLabel(const RendererContext &, const Rectangle &) override;
			float getWidth(const RendererContext &, float height) const override;
	};
}
