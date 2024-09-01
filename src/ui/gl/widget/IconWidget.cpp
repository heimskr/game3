#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/Texture.h"
#include "ui/gl/widget/IconWidget.h"

namespace Game3 {
	IconWidget::IconWidget(float scale):
		Widget(scale) {}

	void IconWidget::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		if (fixedHeight > 0)
			height = fixedHeight;

		const float original_width = width;
		const float original_height = height;
		float dummy{};
		measure(renderers, Orientation::Horizontal, original_width, original_height, dummy, width);
		measure(renderers, Orientation::Vertical,   original_width, original_height, dummy, height);

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

	SizeRequestMode IconWidget::getRequestMode() const {
		return SizeRequestMode::ConstantSize;
	}

	void IconWidget::measure(const RendererContext &, Orientation orientation, float, float, float &minimum, float &natural) {
		float size{};

		if (orientation == Orientation::Horizontal) {
			if (fixedWidth)
				size = fixedWidth;
			else if (iconTexture)
				size = scale * iconTexture->width;
			else
				size = scale * 16;
		} else {
			if (fixedHeight)
				size = fixedHeight;
			else if (iconTexture)
				size = scale * iconTexture->height;
			else
				size = scale * 16;
		}

		minimum = natural = size;
	}

	void IconWidget::setIconTexture(TexturePtr new_icon_texture) {
		iconTexture = std::move(new_icon_texture);
		if (iconTexture)
			iconTexture->init();
	}
}
