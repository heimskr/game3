#include "Constants.h"
#include "graphics/Tileset.h"
#include "graphics/Util.h"
#include "realm/Realm.h"
#include "ui/Window.h"

namespace Game3 {
	glm::mat4 makeMapModel(const RenderOptions &options, int texture_width, int texture_height, const Tileset &tileset, const Window &window) {
		const auto tile_size  = tileset.getTileSize();
		const auto map_length = CHUNK_SIZE * REALM_DIAMETER;
		auto x = options.x;
		auto y = options.y;

		x *= tile_size * window.scale / 2.;
		y *= tile_size * window.scale / 2.;

		const double x_scale = options.scaleX;
		const double y_scale = options.scaleY;
		auto viewport_x = options.viewportX;
		auto viewport_y = options.viewportY;

		if (viewport_x < 0)
			viewport_x *= -window.getWidth();

		if (viewport_y < 0)
			viewport_y *= -window.getHeight();

		x += viewport_x / 2.;
		x -= map_length * tile_size * window.scale / 4.; // TODO: the math here is a little sus... things might cancel out
		x += window.center.first * window.scale * tile_size / 2.;

		y += viewport_y / 2.;
		y -= map_length * tile_size * window.scale / 4.;
		y += window.center.second * window.scale * tile_size / 2.;

		glm::mat4 model = glm::mat4(1.);

		// first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
		model = glm::translate(model, glm::vec3(x - options.offsetX * window.scale * x_scale / 2.0, y - options.offsetY * window.scale * y_scale / 2.0, 0.));

		if (options.angle != 0) {
			const float xs = texture_width * x_scale * window.scale / 4.;
			const float ys = texture_height * y_scale * window.scale / 4.;
			model = glm::translate(model, glm::vec3(xs, ys, 0.)); // move origin of rotation to center of quad
			model = glm::rotate(model, static_cast<float>(glm::radians(options.angle)), glm::vec3(0., 0., 1.)); // then rotate
			model = glm::translate(model, glm::vec3(-xs, -ys, 0.)); // move origin back
		}

		return glm::scale(model, glm::vec3(texture_width * x_scale * window.scale / 2., texture_height * y_scale * window.scale / 2., 2.)); // last scale
	}
}