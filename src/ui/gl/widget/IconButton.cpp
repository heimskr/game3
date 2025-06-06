#include "graphics/Texture.h"
#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "ui/gl/widget/IconButton.h"

namespace Game3 {
	void IconButton::setIconTexture(TexturePtr new_icon_texture) {
		iconTexture = std::move(new_icon_texture);
		if (iconTexture) {
			iconTexture->init();
		}
	}

	void IconButton::renderLabel(const RendererContext &renderers, const Rectangle &rectangle) {
		if (!iconTexture) {
			return;
		}

		const auto scale = getScale();
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

	float IconButton::getWidth(const RendererContext &, float height) const {
		if (!iconTexture) {
			return -1;
		}

		height -= 2 * selfScale;
		return static_cast<float>(iconTexture->width) / static_cast<float>(iconTexture->height) * height;
	}
}
