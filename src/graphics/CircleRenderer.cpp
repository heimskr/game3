#include <cmath>
#include <iostream>

#include "graphics/Shader.h"

#include <glm/gtc/matrix_transform.hpp>

#include "game/ClientGame.h"
#include "game/TileProvider.h"
#include "graphics/GL.h"
#include "graphics/CircleRenderer.h"
#include "graphics/RenderOptions.h"
#include "graphics/Tileset.h"
#include "realm/Realm.h"
#include "ui/Canvas.h"
#include "util/FS.h"
#include "util/Util.h"

namespace Game3 {
	namespace {
		const std::string & rectangleFrag() { static auto out = readFile("resources/circle.frag"); return out; }
		const std::string & rectangleVert() { static auto out = readFile("resources/circle.vert"); return out; }
		constexpr GLenum BLEND_MODE = GL_ONE;
	}

	CircleRenderer::CircleRenderer(Canvas &canvas_): shader("CircleRenderer"), canvas(canvas_) {
		shader.init(rectangleVert(), rectangleFrag()); CHECKGL
		initRenderData(32); CHECKGL
	}

	CircleRenderer::~CircleRenderer() {
		reset();
	}

	void CircleRenderer::reset() {
		if (initializedTo <= 0) {
			glDeleteVertexArrays(1, &quadVAO); CHECKGL
			quadVAO = 0;
			initializedTo = 0;
		}
	}

	void CircleRenderer::update(int width, int height) {
		if (width != backbufferWidth || height != backbufferHeight) {
			HasBackbuffer::update(width, height);
			projection = glm::ortho(0., double(width), double(height), 0., -1., 1.);
			shader.bind(); CHECKGL
			shader.set("projection", projection); CHECKGL
		}
	}

	void CircleRenderer::drawOnMap(const RenderOptions &options, float cutoff) {
		if (!isInitialized())
			return;

		auto width  = options.sizeX * 16.;
		auto height = options.sizeY * 16.;
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

		glm::dmat4 model(1.);
		// first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
		model = glm::translate(model, glm::dvec3(x, y, 0.));
		if (angle != 0) {
			model = glm::translate(model, glm::dvec3(.5 * width, .5 * height, 0.)); // move origin of rotation to center of quad
			model = glm::rotate(model, glm::radians(angle), glm::dvec3(0., 0., 1.)); // then rotate
			model = glm::translate(model, glm::dvec3(-.5 * width, -.5 * height, 0.)); // move origin back
		}
		model = glm::scale(model, glm::dvec3(width * canvas.scale / 2., height * canvas.scale / 2., 1.)); // last scale

		shader.set("model", model); CHECKGL
		shader.set("circleColor", options.color); CHECKGL
		shader.set("cutoff", cutoff);

		glEnable(GL_BLEND); CHECKGL
		glBlendFunc(GL_SRC_ALPHA, BLEND_MODE); CHECKGL
		glBindVertexArray(quadVAO); CHECKGL
		glDrawArrays(GL_TRIANGLES, 0, initializedTo * 3); CHECKGL
		glBindVertexArray(0); CHECKGL
	}

	void CircleRenderer::drawOnScreen(const Color &color, double x, double y, double width, double height, float cutoff, double angle) {
		if (!isInitialized())
			return;

		y = backbufferHeight - y;

		shader.bind(); CHECKGL

		glm::dmat4 model(1.);
		// first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
		model = glm::translate(model, glm::dvec3(x, y, 0.));
		if (angle != 0) {
			model = glm::translate(model, glm::dvec3(.5 * width, .5 * height, 0.)); // move origin of rotation to center of quad
			model = glm::rotate(model, glm::radians(angle), glm::dvec3(0., 0., 1.)); // then rotate
			model = glm::translate(model, glm::dvec3(-.5 * width, -.5 * height, 0.)); // move origin back
		}
		model = glm::scale(model, glm::dvec3(width, height, 1.)); // last scale

		shader.set("model", model); CHECKGL
		shader.set("circleColor", color); CHECKGL
		shader.set("cutoff", cutoff);

		glEnable(GL_BLEND); CHECKGL
		glBlendFunc(GL_SRC_ALPHA, BLEND_MODE); CHECKGL
		glBindVertexArray(quadVAO); CHECKGL
		glDrawArrays(GL_TRIANGLES, 0, initializedTo * 3); CHECKGL
		glBindVertexArray(0); CHECKGL
	}

	void CircleRenderer::initRenderData(int sides) {
		GLuint vbo;

		std::shared_lock<DefaultMutex> shared_lock;
		const std::vector<double> &vertices = getVertices(sides, shared_lock);

		glGenVertexArrays(1, &quadVAO); CHECKGL
		glGenBuffers(1, &vbo); CHECKGL

		GLint old_abb = 0;
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &old_abb); CHECKGL

		glBindBuffer(GL_ARRAY_BUFFER, vbo); CHECKGL
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(double), vertices.data(), GL_STATIC_DRAW); CHECKGL

		glBindVertexArray(quadVAO); CHECKGL
		glEnableVertexAttribArray(0); CHECKGL
		GL::vertexAttribPointer<double>(0, 2, GL_FALSE, 2 * sizeof(double), nullptr); CHECKGL
		glBindBuffer(GL_ARRAY_BUFFER, old_abb); CHECKGL
		glBindVertexArray(0); CHECKGL
		initializedTo = sides;
	}

	const std::vector<double> & CircleRenderer::getVertices(int sides, std::shared_lock<DefaultMutex> &shared_lock) {
		shared_lock = vertexMap.sharedLock();
		if (auto iter = vertexMap.find(sides); iter != vertexMap.end())
			return iter->second;

		shared_lock = {};
		auto unique_lock = vertexMap.uniqueLock();
		if (auto iter = vertexMap.find(sides); iter != vertexMap.end()) {
			unique_lock = {};
			shared_lock = vertexMap.sharedLock();
			return iter->second;
		}

		std::vector<double> &vertices = vertexMap[sides];
		for (int i = 0; i < sides; ++i) {
			const double rad1 = 2. * M_PI * i / sides;
			const double rad2 = 2. * M_PI * (i + 1) / sides;
			const double x1 = std::cos(rad1);
			const double y1 = std::sin(rad1);
			const double x2 = std::cos(rad2);
			const double y2 = std::sin(rad2);
			vertices.push_back(0.);
			vertices.push_back(0.);
			vertices.push_back(x1);
			vertices.push_back(y1);
			vertices.push_back(x2);
			vertices.push_back(y2);
		}

		unique_lock = {};
		shared_lock = vertexMap.sharedLock();
		return vertices;
	}

	bool CircleRenderer::isInitialized() const {
		return 0 < initializedTo;
	}
}
