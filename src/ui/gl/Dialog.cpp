#include "graphics/RendererContext.h"
#include "graphics/SingleSpriteRenderer.h"
#include "graphics/Texture.h"
#include "ui/gl/Dialog.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	void Dialog::drawFrame(UIContext &ui, RendererContext &renderers, double scale, bool alpha, const std::array<std::string_view, 8> &pieces) {
		Rectangle rectangle = ui.scissorStack.getTop();
		SingleSpriteRenderer &single = renderers.singleSprite;

		TexturePtr top_left     = cacheTexture(pieces[0], alpha);
		TexturePtr top          = cacheTexture(pieces[1], alpha);
		TexturePtr top_right    = cacheTexture(pieces[2], alpha);
		TexturePtr right        = cacheTexture(pieces[3], alpha);
		TexturePtr bottom_right = cacheTexture(pieces[4], alpha);
		TexturePtr bottom       = cacheTexture(pieces[5], alpha);
		TexturePtr bottom_left  = cacheTexture(pieces[6], alpha);
		TexturePtr left         = cacheTexture(pieces[7], alpha);

		single.drawOnScreen(top_left, RenderOptions{
			.x = 0,
			.y = double(rectangle.height),
			.sizeX = -1,
			.sizeY = -1,
			.scaleX = scale,
			.scaleY = scale,
			.invertY = false,
		});

		single.drawOnScreen(top, RenderOptions{
			.x = top_left->width * scale,
			.y = double(rectangle.height),
			.sizeX = rectangle.width - (top_left->width + top_right->width) * scale,
			.sizeY = -1,
			.scaleX = scale,
			.scaleY = scale,
			.invertY = false,
			.wrapMode = GL_REPEAT,
		});

		single.drawOnScreen(top_right, RenderOptions{
			.x = rectangle.width - top_right->width * scale,
			.y = double(rectangle.height),
			.sizeX = -1,
			.sizeY = -1,
			.scaleX = scale,
			.scaleY = scale,
			.invertY = false,
		});

		single.drawOnScreen(left, RenderOptions{
			.x = 0,
			.y = rectangle.height - top_left->height * scale,
			.sizeX = -1,
			.sizeY = rectangle.height - (top_left->height + bottom_left->height) * scale,
			.scaleX = scale,
			.scaleY = scale,
			.invertY = false,
			.wrapMode = GL_REPEAT,
		});

		single.drawOnScreen(right, RenderOptions{
			.x = rectangle.width - top_right->width * scale,
			.y = rectangle.height - top_right->height * scale,
			.sizeX = -1,
			.sizeY = rectangle.height - (top_right->height + bottom_right->height) * scale,
			.scaleX = scale,
			.scaleY = scale,
			.invertY = false,
			.wrapMode = GL_REPEAT,
		});

		single.drawOnScreen(bottom_left, RenderOptions{
			.x = 0,
			.y = bottom_left->height * scale,
			.sizeX = -1,
			.sizeY = -1,
			.scaleX = scale,
			.scaleY = scale,
			.invertY = false,
		});

		single.drawOnScreen(bottom, RenderOptions{
			.x = bottom_left->width * scale,
			.y = bottom->height * scale,
			.sizeX = rectangle.width - (bottom_left->width + bottom_right->width) * scale,
			.sizeY = -1,
			.scaleX = scale,
			.scaleY = scale,
			.invertY = false,
			.wrapMode = GL_REPEAT,
		});

		single.drawOnScreen(bottom_right, RenderOptions{
			.x = rectangle.width - bottom_right->width * scale,
			.y = bottom_right->height * scale,
			.sizeX = -1,
			.sizeY = -1,
			.scaleX = scale,
			.scaleY = scale,
			.invertY = false,
		});
	}
}
