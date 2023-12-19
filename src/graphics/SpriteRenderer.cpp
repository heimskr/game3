#include "graphics/SpriteRenderer.h"
#include "ui/Canvas.h"

namespace Game3 {
	SpriteRenderer::SpriteRenderer(Canvas &canvas_):
		canvas(&canvas_) {}

	void SpriteRenderer::update(const Canvas &canvas) {
		update(canvas.getWidth(), canvas.getHeight());
	}

	void SpriteRenderer::drawOnMap(const std::shared_ptr<Texture> &texture, double x, double y) {
		drawOnMap(texture, x, y, 1.f, 0.f, 1.f);
	}

	void SpriteRenderer::drawOnMap(const std::shared_ptr<Texture> &texture) {
		drawOnMap(texture, RenderOptions{});
	}
}
