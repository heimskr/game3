#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/Texture.h"
#include "ui/gl/widget/Icon.h"
#include "ui/gl/widget/Tooltip.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	Icon::Icon(UIContext &ui, float scale):
		Widget(ui, scale) {}

	void Icon::render(const RendererContext &renderers, float x, float y, float width, float height) {
		if (fixedHeight >= 0) {
			height = fixedHeight;
		}

		const float original_width = width;
		const float original_height = height;
		float dummy{};
		measure(renderers, Orientation::Horizontal, original_width, original_height, dummy, width);
		measure(renderers, Orientation::Vertical,   original_width, original_height, dummy, height);

		Widget::render(renderers, x, y, width, height);

		if (!iconTexture || shouldCull()) {
			return;
		}

		renderers.singleSprite.drawOnScreen(iconTexture, RenderOptions{
			.x = x,
			.y = y,
			.sizeX = -1,
			.sizeY = -1,
			.scaleX = width / iconTexture->width,
			.scaleY = height / iconTexture->height,
			.invertY = false,
		});

		std::shared_ptr<Tooltip> tooltip = ui.getTooltip();

		if (tooltipText && !ui.anyDragUpdaters() && ui.checkMouse(lastRectangle)) {
			if (!tooltip->wasUpdatedBy(*this) || tooltipTextChanged) {
				tooltipTextChanged = false;
				tooltip->setText(*tooltipText);
			}
			tooltip->setRegion(lastRectangle);
			tooltip->show(*this);
		} else {
			tooltip->hide(*this);
		}
	}

	SizeRequestMode Icon::getRequestMode() const {
		return SizeRequestMode::ConstantSize;
	}

	void Icon::measure(const RendererContext &, Orientation orientation, float, float, float &minimum, float &natural) {
		float size{};

		if (orientation == Orientation::Horizontal) {
			if (fixedWidth >= 0) {
				size = fixedWidth;
			} else if (iconTexture) {
				size = selfScale * iconTexture->width;
			} else {
				size = selfScale * 16;
			}
		} else {
			if (fixedHeight >= 0) {
				size = fixedHeight;
			} else if (iconTexture) {
				size = selfScale * iconTexture->height;
			} else {
				size = selfScale * 16;
			}
		}

		minimum = natural = size;
	}

	void Icon::setIconTexture(TexturePtr new_icon_texture) {
		assert(new_icon_texture != nullptr);
		iconTexture = std::move(new_icon_texture);
		if (iconTexture) {
			iconTexture->init();
		}
	}
}
