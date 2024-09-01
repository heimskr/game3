#pragma once

#include "ui/gl/widget/ButtonWidget.h"

namespace Game3 {
	class IconButtonWidget: public ButtonWidget {
		public:
			using ButtonWidget::ButtonWidget;

			void setIconTexture(TexturePtr);

		protected:
			TexturePtr iconTexture;

			void renderLabel(UIContext &, const RendererContext &, const Rectangle &) override;
			float getWidth(const RendererContext &, float height) const override;
	};
}
