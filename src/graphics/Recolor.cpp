// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.ull_source/sprite_renderer.cpp

#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/Tileset.h"
#include "game/ClientGame.h"
#include "graphics/GL.h"
#include "graphics/Recolor.h"
#include "ui/Canvas.h"
#include "util/FS.h"
#include "util/Util.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Game3 {
	namespace {
		const std::string & recolorFrag() { static auto out = readFile("resources/recolor.frag"); return out; }
		const std::string & spriteVert()  { static auto out = readFile("resources/sprite.vert");  return out; }
	}

	Recolor::Recolor(Canvas &canvas_): canvas(&canvas_), shader("Recolor") {
		shader.init(spriteVert(), recolorFrag());
		initRenderData();
	}

	Recolor::Recolor(Recolor &&other): Recolor(*other.canvas) {
		other.canvas = nullptr;
		shader = std::move(other.shader);
		quadVAO = other.quadVAO;
		initialized = other.initialized;
		backbufferWidth = other.backbufferWidth;
		backbufferHeight = other.backbufferHeight;
		other.quadVAO = 0;
		other.initialized = false;
		other.backbufferWidth = -1;
		other.backbufferHeight = -1;
	}

	Recolor::~Recolor() {
		remove();
	}

	void Recolor::remove() {
		if (initialized) {
			glDeleteVertexArrays(1, &quadVAO); CHECKGL
			quadVAO = 0;
			initialized = false;
		}
	}

	Recolor & Recolor::operator=(Recolor &&other) {
		canvas = other.canvas;
		other.canvas = nullptr;
		shader.reset();
		shader = std::move(other.shader);
		quadVAO = other.quadVAO;
		initialized = other.initialized;
		backbufferWidth = other.backbufferWidth;
		backbufferHeight = other.backbufferHeight;
		other.quadVAO = 0;
		other.initialized = false;
		other.backbufferWidth = -1;
		other.backbufferHeight = -1;
		return *this;
	}

	void Recolor::update(const Canvas &canvas) {
		centerX = canvas.center.first;
		centerY = canvas.center.second;
		update(canvas.getWidth(), canvas.getHeight());
	}

	void Recolor::update(int width, int height) {
		if (width != backbufferWidth || height != backbufferHeight) {
			HasBackbuffer::update(width, height);
			glm::mat4 projection = glm::ortho(0., double(width), double(height), 0., -1., 1.);
			shader.bind();
			shader.set("projection", projection);
		}
	}

	void Recolor::drawOnMap(const std::shared_ptr<Texture> &texture, const std::shared_ptr<Texture> &mask, const RenderOptions &options, float hue, float saturation, float value_multiplier) {
		if (!initialized)
			return;

		auto size_x = options.sizeX;
		auto size_y = options.sizeY;
		auto x = options.x;
		auto y = options.y;

		if (size_x < 0)
			size_x = texture->width;
		if (size_y < 0)
			size_y = texture->height;

		assert(canvas != nullptr);
		const auto [center_x, center_y] = canvas->center;
		RealmPtr realm = canvas->game->getActiveRealm();
		TileProvider &provider = realm->tileProvider;
		TilesetPtr tileset     = provider.getTileset(*canvas->game);
		const auto tile_size   = tileset->getTileSize();
		const auto map_length  = CHUNK_SIZE * REALM_DIAMETER;

		x *= tile_size * canvas->scale / 2.;
		y *= tile_size * canvas->scale / 2.;

		x += backbufferWidth / 2.;
		x -= map_length * tile_size * canvas->scale / canvas->magic * 2.; // TODO: the math here is a little sus... things might cancel out
		x += center_x * canvas->scale * tile_size / 2.;

		y += backbufferHeight / 2.;
		y -= map_length * tile_size * canvas->scale / canvas->magic * 2.;
		y += center_y * canvas->scale * tile_size / 2.;

		shader.bind();

		glm::mat4 model = glm::mat4(1.);
		// first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
		model = glm::translate(model, glm::vec3(x - options.offsetX * canvas->scale * options.scaleX, y - options.offsetY * canvas->scale * options.scaleY, 0.));
		if (options.angle != 0) {
			model = glm::translate(model, glm::vec3(.5 * texture->width, .5 * texture->height, 0.)); // move origin of rotation to center of quad
			model = glm::rotate(model, float(glm::radians(options.angle)), glm::vec3(0., 0., 1.)); // then rotate
			model = glm::translate(model, glm::vec3(-.5 * texture->width, -.5 * texture->height, 0.)); // move origin back
		}
		model = glm::scale(model, glm::vec3(texture->width * options.scaleX * canvas->scale / 2., texture->height * options.scaleY * canvas->scale / 2., 2.)); // last scale

		shader.set("model", model);
		shader.set("spriteColor", options.color);
		const double multiplier_x = 2. / texture->width;
		const double multiplier_y = 2. / texture->height;
		shader.set("texturePosition", options.offsetX * multiplier_x, options.offsetY * multiplier_y, size_x / texture->width, size_y / texture->width);
		shader.set("hue", hue);
		shader.set("saturation", saturation);
		shader.set("valueMultiplier", value_multiplier);
		shader.set("sprite", 0);
		shader.set("mask", 1);

		texture->bind(0);
		mask->bind(1);

		glEnable(GL_BLEND); CHECKGL
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); CHECKGL
		glBindVertexArray(quadVAO); CHECKGL
		glDrawArrays(GL_TRIANGLES, 0, 6); CHECKGL
		glBindVertexArray(0); CHECKGL
	}

	void Recolor::reset() {
		shader.init(spriteVert(), recolorFrag());
		initRenderData();
	}

	void Recolor::initRenderData() {
		if (initialized) {
			glDeleteVertexArrays(1, &quadVAO); CHECKGL
		}

		unsigned int vbo;
		static const float vertices[] {
			// pos    // tex
			0., 1., 0., 1.,
			1., 0., 1., 0.,
			0., 0., 0., 0.,

			0., 1., 0., 1.,
			1., 1., 1., 1.,
			1., 0., 1., 0.
		};

		glGenVertexArrays(1, &quadVAO); CHECKGL
		glGenBuffers(1, &vbo); CHECKGL

		glBindBuffer(GL_ARRAY_BUFFER, vbo); CHECKGL
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); CHECKGL

		glBindVertexArray(quadVAO); CHECKGL
		glEnableVertexAttribArray(0); CHECKGL
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr); CHECKGL
		glBindBuffer(GL_ARRAY_BUFFER, 0); CHECKGL
		glBindVertexArray(0); CHECKGL
		initialized = true;
	}

	void Recolor::setupShader(int texture_width, int texture_height, const RenderOptions &options) {
		glm::mat4 model = glm::mat4(1.);
		// first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
		model = glm::translate(model, glm::vec3(options.x - options.offsetX * 2. * options.scaleX, options.y - options.offsetY * 2. * options.scaleY, 0.f));
		if (options.invertY)
			model = glm::scale(model, glm::vec3(1., -1., 1.));
		if (options.angle != 0)  {
			model = glm::translate(model, glm::vec3(.5 * texture_width, .5 * texture_height, 0.f));
			model = glm::rotate   (model, float(glm::radians(options.angle)), glm::vec3(0.f, 0.f, 1.f));
			model = glm::translate(model, glm::vec3(-.5 * texture_width, -.5 * texture_height, 0.f));
		}
		model = glm::scale(model, glm::vec3(texture_width * options.scaleX, texture_height * options.scaleY, 1.f));

		shader.bind();
		shader.set("model", model);
		shader.set("spriteColor", options.color.red, options.color.green, options.color.blue, options.color.alpha);
		const double multiplier = 2.;
		const double multiplier_x = multiplier / texture_width;
		const double multiplier_y = multiplier / texture_height;
		shader.set("texturePosition", options.offsetX * multiplier_x, options.offsetY * multiplier_y, options.sizeX / texture_width, options.sizeY / texture_height);
		shader.set("sprite", 0);
		shader.set("mask", 1);
	}
}
