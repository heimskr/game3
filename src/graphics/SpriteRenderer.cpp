#include "graphics/SpriteRenderer.h"
#include "ui/Canvas.h"

namespace Game3 {
	SpriteRenderer::SpriteRenderer(Canvas &canvas_):
		canvas(&canvas_) {}

	void SpriteRenderer::drawOnMap(const std::shared_ptr<Texture> &texture, double x, double y) {
		drawOnMap(texture, x, y, 1.f, 0.f, 1.f);
	}

	void SpriteRenderer::drawOnMap(const std::shared_ptr<Texture> &texture) {
		drawOnMap(texture, RenderOptions{});
	}

	void SpriteRenderer::drawOnScreen(GL::Texture &, const RenderOptions &) {
		throw std::logic_error("SpriteRenderer::drawOnScreen (GL::Texture overload) unimplemented");
	}

	void SpriteRenderer::drawOnScreen(const std::shared_ptr<Texture> &, const RenderOptions &) {
		throw std::logic_error("SpriteRenderer::drawOnScreen (TexturePtr overload) unimplemented");
	}
}
