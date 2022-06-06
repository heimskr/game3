#include <unordered_set>

#include <libnoise/noise.h>

#include "ui/Canvas.h"
#include "ui/MainWindow.h"

#include "game/Game.h"
#include "menu/Menu.h"
#include "util/Timer.h"
#include "util/Util.h"

#include "Tiles.h"

namespace Game3 {
	Canvas::Canvas(MainWindow &window_): window(window_) {
		magic = 16 / 2;
	}

	void Canvas::drawGL() {
		if (game) {
			game->tick();
			spriteRenderer.update(width(), height());
			rectangleRenderer.update(width(), height());

			if (game->activeRealm) {
				auto &realm = *game->activeRealm;
				realm.render(width(), height(), center, scale, spriteRenderer);
			}

			if (game->menu)
				game->menu->render(*game, *this, context);
		}
	}

	int Canvas::width() {
		return window.glArea.get_width();
	}

	int Canvas::height() {
		return window.glArea.get_height();
	}

	// bool Canvas::mouseButtonEvent(const nanogui::Vector2i &p, int button, bool down, int modifiers) {
	// 	if (nanogui::GLCanvas::mouseButtonEvent(p, button, down, modifiers))
	// 		return true;

	// 	if (!down && game && game->activeRealm) {
	// 		const auto &realm = *game->activeRealm;
	// 		const auto &tilemap = realm.tilemap1;

	// 		float fx = p.x();
	// 		float fy = p.y() - HEADER_HEIGHT / 2.f;

	// 		fx -= width() / 2.f - (tilemap->width * tilemap->tileSize / 4.f) * scale + center.x() * magic * scale;
	// 		fx /= tilemap->tileSize * scale / 2.f;

	// 		fy -= height() / 2.f - (tilemap->height * tilemap->tileSize / 4.f) * scale + center.y() * magic * scale;
	// 		fy /= tilemap->tileSize * scale / 2.f;

	// 		int x = fx;
	// 		int y = fy;

	// 		(void) x;
	// 		(void) y;
	// 		// std::cerr << x << ", " << y << '\n';

	// 		return true;
	// 	}

	// 	return false;
	// }
}
