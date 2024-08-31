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

	void IconButtonWidget::renderLabel(UIContext &, const RendererContext &renderers, float width, float height) {
		if (!iconTexture)
			return;

		width -= scale;
		height -= scale;

		renderers.singleSprite.drawOnScreen(iconTexture, RenderOptions{
			.x = scale / 2,
			.y = scale / 2,
			.sizeX = -1,
			.sizeY = -1,
			.scaleX = width  / iconTexture->width,
			.scaleY = height / iconTexture->height,
			.invertY = false,
		});
	}

	void IconButtonWidget::adjustWidth(const RendererContext &, float &width, float height) const {
		if (!iconTexture)
			return;

		height -= 2 * scale;
		width = static_cast<float>(iconTexture->width) / static_cast<float>(iconTexture->height) * height;
	}
}
