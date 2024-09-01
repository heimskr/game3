#include "graphics/Texture.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "ui/gl/widget/IconButtonWidget.h"

namespace Game3 {
	void IconButtonWidget::setIconTexture(TexturePtr new_icon_texture) {
		iconTexture = std::move(new_icon_texture);
		if (iconTexture)
			iconTexture->init();
	}

	void IconButtonWidget::renderLabel(UIContext &, const RendererContext &renderers, const Rectangle &rectangle) {
		if (!iconTexture)
			return;

		const float width  = rectangle.width  - scale;
		const float height = rectangle.height - scale;

		renderers.singleSprite.drawOnScreen(iconTexture, RenderOptions{
			.x = rectangle.x + scale / 2,
			.y = rectangle.y + scale / 2,
			.sizeX = -1,
			.sizeY = -1,
			.scaleX = width  / iconTexture->width,
			.scaleY = height / iconTexture->height,
			.invertY = false,
		});
	}

	float IconButtonWidget::getWidth(const RendererContext &, float height) const {
		if (!iconTexture)
			return -1;

		height -= 2 * scale;
		return static_cast<float>(iconTexture->width) / static_cast<float>(iconTexture->height) * height;
	}
}
