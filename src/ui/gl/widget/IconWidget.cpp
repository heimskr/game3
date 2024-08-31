#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/Texture.h"
#include "ui/gl/widget/IconWidget.h"

namespace Game3 {
	IconWidget::IconWidget(float scale):
		Widget(scale) {}

	void IconWidget::setIconTexture(TexturePtr new_icon_texture) {
		iconTexture = std::move(new_icon_texture);
		if (iconTexture)
			iconTexture->init();
	}

	void IconWidget::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		if (fixedHeight > 0)
			height = fixedHeight;

		if (width < 0 && iconTexture)
			width = height * iconTexture->width / iconTexture->height;

		Widget::render(ui, renderers, x, y, width, height);

		if (!iconTexture)
			return;

		renderers.singleSprite.drawOnScreen(iconTexture, RenderOptions{
			.x = x,
			.y = y,
			.sizeX = -1,
			.sizeY = -1,
			.scaleX = width / iconTexture->width,
			.scaleY = height / iconTexture->height,
			.invertY = false,
		});
	}

	float IconWidget::calculateHeight(const RendererContext &, float, float available_height) {
		return fixedHeight > 0? fixedHeight : available_height;
	}
}
