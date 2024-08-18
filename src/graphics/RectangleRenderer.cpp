#include <iostream>

#include "graphics/Shader.h"

#include <glm/gtc/matrix_transform.hpp>

#include "game/ClientGame.h"
#include "game/TileProvider.h"
#include "graphics/GL.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RenderOptions.h"
#include "graphics/Tileset.h"
#include "realm/Realm.h"
#include "ui/Canvas.h"
#include "util/FS.h"
#include "util/Util.h"

namespace Game3 {
	namespace {
		const std::string & rectangleFrag() { static auto out = readFile("resources/rectangle.frag"); return out; }
		const std::string & rectangleVert() { static auto out = readFile("resources/rectangle.vert"); return out; }
	}

	RectangleRenderer::RectangleRenderer(Canvas &canvas_): shader("RectangleRenderer"), canvas(canvas_) {
		shader.init(rectangleVert(), rectangleFrag()); CHECKGL
		initRenderData(); CHECKGL
	}

	RectangleRenderer::~RectangleRenderer() {
		reset();
	}

	void RectangleRenderer::reset() {
		if (initialized) {
			glDeleteVertexArrays(1, &quadVAO); CHECKGL
			quadVAO = 0;
			initialized = false;
		}
	}

	void RectangleRenderer::update(int width, int height) {
		if (width != backbufferWidth || height != backbufferHeight) {
			backbufferWidth = width;
			backbufferHeight = height;
			projection = glm::ortho(0.f, float(width), float(height), 0.f, -1.f, 1.f);
			shader.bind(); CHECKGL
			shader.set("projection", projection); CHECKGL
		}
	}

	void RectangleRenderer::drawOnMap(const RenderOptions &options) {
		if (!initialized)
			return;

		auto width  = options.sizeX * 16;
		auto height = options.sizeY * 16;
		auto angle  = options.angle;
		auto x = options.x;
		auto y = options.y;

		const auto [center_x, center_y] = canvas.center;
		RealmPtr realm = canvas.game->getActiveRealm();
		TileProvider &provider = realm->tileProvider;
		TilesetPtr tileset     = provider.getTileset(*canvas.game);
		const auto tile_size   = tileset->getTileSize();
		const auto map_length  = CHUNK_SIZE * REALM_DIAMETER;

		x *= tile_size * canvas.scale / 2.;
		y *= tile_size * canvas.scale / 2.;

		x += backbufferWidth / 2.;
		x -= map_length * tile_size * canvas.scale / canvas.magic * 2.; // TODO: the math here is a little sus... things might cancel out
		x += center_x * canvas.scale * tile_size / 2.;

		y += backbufferHeight / 2.;
		y -= map_length * tile_size * canvas.scale / canvas.magic * 2.;
		y += center_y * canvas.scale * tile_size / 2.;

		shader.bind(); CHECKGL

		glm::mat4 model = glm::mat4(1.f);
		// first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
		model = glm::translate(model, glm::vec3(x, y, 0.f));
		if (angle != 0) {
			model = glm::translate(model, glm::vec3(0.5f * width, 0.5f * height, 0.f)); // move origin of rotation to center of quad
			model = glm::rotate(model, float(glm::radians(angle)), glm::vec3(0.f, 0.f, 1.f)); // then rotate
			model = glm::translate(model, glm::vec3(-0.5f * width, -0.5f * height, 0.f)); // move origin back
		}
		model = glm::scale(model, glm::vec3(width * canvas.scale / 2., height * canvas.scale / 2., 1.f)); // last scale

		shader.set("model", model); CHECKGL
		shader.set("rectColor", options.color); CHECKGL

		glBindVertexArray(quadVAO); CHECKGL
		glDrawArrays(GL_TRIANGLES, 0, 6); CHECKGL
		glBindVertexArray(0); CHECKGL
	}

	void RectangleRenderer::drawOnScreen(const Color &color, float x, float y, float width, float height, float angle) {
		if (!initialized)
			return;

		shader.bind(); CHECKGL

		glm::mat4 model = glm::mat4(1.f);
		// first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
		model = glm::translate(model, glm::vec3(x, y, 0.f));
		if (angle != 0) {
			model = glm::translate(model, glm::vec3(0.5f * width, 0.5f * height, 0.f)); // move origin of rotation to center of quad
			model = glm::rotate(model, glm::radians(angle), glm::vec3(0.f, 0.f, 1.f)); // then rotate
			model = glm::translate(model, glm::vec3(-0.5f * width, -0.5f * height, 0.f)); // move origin back
		}
		model = glm::scale(model, glm::vec3(width, height, 1.f)); // last scale

		shader.set("model", model); CHECKGL
		shader.set("rectColor", color); CHECKGL

		glBindVertexArray(quadVAO); CHECKGL
		glDrawArrays(GL_TRIANGLES, 0, 6); CHECKGL
		glBindVertexArray(0); CHECKGL
	}

	void RectangleRenderer::operator()(const Color &color, float x, float y, float width, float height, float angle) {
		drawOnScreen(color, x, y, width, height, angle);
	}

	void RectangleRenderer::initRenderData() {
		GLuint vbo;

		static const float vertices[] {
			0.f, 1.f, 0.f, 1.f,
			1.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 0.f,

			0.f, 1.f, 0.f, 1.f,
			1.f, 1.f, 1.f, 1.f,
			1.f, 0.f, 1.f, 0.f,
		};

		glGenVertexArrays(1, &quadVAO); CHECKGL
		glGenBuffers(1, &vbo); CHECKGL

		GLint old_abb = 0;
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &old_abb); CHECKGL

		glBindBuffer(GL_ARRAY_BUFFER, vbo); CHECKGL
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); CHECKGL

		glBindVertexArray(quadVAO); CHECKGL
		glEnableVertexAttribArray(0); CHECKGL
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr); CHECKGL
		glBindBuffer(GL_ARRAY_BUFFER, old_abb); CHECKGL
		glBindVertexArray(0); CHECKGL
		initialized = true;
	}
}
