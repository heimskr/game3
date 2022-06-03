// Contains code from nanogui and from LearnOpenGL (https://github.com/JoeyDeVries/LearnOpenGL)

#include <unordered_set>

#include <libnoise/noise.h>

#include "ui/Canvas.h"

#include "MarchingSquares.h"
#include "resources.h"
#include "game/Game.h"
#include "util/Timer.h"
#include "util/Util.h"

namespace Game3 {

	Canvas::Canvas(nanogui::Widget *parent_): GLCanvas(parent_) {
		setBackgroundColor({66, 172, 175, 255});
		int scale = 16;
		magic = scale / 2;
	}

	void Canvas::draw(NVGcontext *context_) {
		nanogui::GLCanvas::draw(context_);
		if (context_ != nullptr && context != context_) {
			context = context_;
			font = nvgCreateFont(context, "FreeSans", "resources/FreeSans.ttf");
		}
	}

	void Canvas::drawGL() {
		if (game && game->activeRealm) {
			auto &realm = *game->activeRealm;
			realm.render(width(), height(), center, scale);
		}
	}

	bool Canvas::scrollEvent(const nanogui::Vector2i &p, const nanogui::Vector2f &rel) {
		if (nanogui::GLCanvas::scrollEvent(p, rel))
			return true;

		if (rel.y() == 1)
			scale *= 1.06f;
		else if (rel.y() == -1)
			scale /= 1.06f;

		return true;
	}

	bool Canvas::mouseDragEvent(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button, int modifiers) {
		if (nanogui::GLCanvas::mouseDragEvent(p, rel, button, modifiers))
			return true;

		if (button == 1) {
			center.x() += rel.x() / (magic * scale);
			center.y() += rel.y() / (magic * scale);
			return true;
		}

		return false;
	}

	bool Canvas::mouseButtonEvent(const nanogui::Vector2i &p, int button, bool down, int modifiers) {
		if (nanogui::GLCanvas::mouseButtonEvent(p, button, down, modifiers))
			return true;

		if (!down && game && game->activeRealm) {
			const auto &realm = *game->activeRealm;
			const auto &tilemap = realm.tilemap1;

			float fx = p.x();
			float fy = p.y() - HEADER_HEIGHT / 2.f;

			fx -= width() / 2.f - (tilemap->width * tilemap->tileSize / 4.f) * scale + center.x() * magic * scale;
			fx /= tilemap->tileSize * scale / 2.f;

			fy -= height() / 2.f - (tilemap->height * tilemap->tileSize / 4.f) * scale + center.y() * magic * scale;
			fy /= tilemap->tileSize * scale / 2.f;

			int x = fx;
			int y = fy;

			std::cerr << x << ", " << y << '\n';

			return true;
		}

		return false;
	}
}
