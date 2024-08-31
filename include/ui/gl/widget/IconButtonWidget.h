#pragma once

#include "ui/gl/widget/ButtonWidget.h"

namespace Game3 {
	class IconButtonWidget: public ButtonWidget {
		public:
			using ButtonWidget::ButtonWidget;

			void setIconTexture(TexturePtr);

		protected:
			TexturePtr iconTexture;

			void renderLabel(UIContext &, const RendererContext &, float width, float height) override;
			void adjustWidth(const RendererContext &, float &width, float height) const override;
	};
}
