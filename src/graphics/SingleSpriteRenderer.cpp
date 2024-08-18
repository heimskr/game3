// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.ull_source/sprite_renderer.cpp

#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/Tileset.h"
#include "game/ClientGame.h"
#include "graphics/GL.h"
#include "graphics/SingleSpriteRenderer.h"
#include "ui/Canvas.h"
#include "util/FS.h"
#include "util/Util.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Game3 {
	namespace {
		const std::string & spriteFrag() { static auto out = readFile("resources/sprite.frag"); return out; }
		const std::string & spriteVert() { static auto out = readFile("resources/sprite.vert"); return out; }
	}

	SingleSpriteRenderer::SingleSpriteRenderer(Canvas &canvas_): SpriteRenderer(canvas_), shader("SingleSpriteRenderer") {
		shader.init(spriteVert(), spriteFrag());
		initRenderData();
	}

	SingleSpriteRenderer::SingleSpriteRenderer(SingleSpriteRenderer &&other): SingleSpriteRenderer(*other.canvas) {
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

	SingleSpriteRenderer::~SingleSpriteRenderer() {
		remove();
	}

	void SingleSpriteRenderer::remove() {
		if (initialized) {
			glDeleteVertexArrays(1, &quadVAO); CHECKGL
			quadVAO = 0;
			initialized = false;
		}
	}

	SingleSpriteRenderer & SingleSpriteRenderer::operator=(SingleSpriteRenderer &&other) {
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

	void SingleSpriteRenderer::update(const Canvas &canvas) {
		centerX = canvas.center.first;
		centerY = canvas.center.second;
		update(canvas.getWidth(), canvas.getHeight());
	}

	void SingleSpriteRenderer::update(int width, int height) {
		if (width != backbufferWidth || height != backbufferHeight) {
			HasBackbuffer::update(width, height);
			glm::mat4 projection = glm::ortho(0., double(width), double(height), 0., -1., 1.);
			shader.bind();
			shader.set("projection", projection);
		}
	}

	void SingleSpriteRenderer::drawOnMap(const TexturePtr &texture, double x, double y, double scale, double angle, double alpha) {
		drawOnMap(texture, RenderOptions {
			.x = x,
			.y = y,
			.sizeX = double(texture->width),
			.sizeY = double(texture->height),
			.scaleX = scale,
			.scaleY = scale,
			.angle = angle,
			.color = {1.f, 1.f, 1.f, float(alpha)},
		});
	}

	void SingleSpriteRenderer::drawOnMap(const TexturePtr &texture, const RenderOptions &options) {
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
		RealmPtr realm = canvas->game->getActiveRealm();
		TileProvider &provider = realm->tileProvider;
		TilesetPtr tileset     = provider.getTileset(*canvas->game);
		const auto tile_size   = tileset->getTileSize();
		const auto map_length  = CHUNK_SIZE * REALM_DIAMETER;

		x *= tile_size * canvas->scale / 2.;
		y *= tile_size * canvas->scale / 2.;

		x += canvas->getWidth() / 2.;
		x -= map_length * tile_size * canvas->scale / canvas->magic * 2.; // TODO: the math here is a little sus... things might cancel out
		x += canvas->center.first * canvas->scale * tile_size / 2.;

		y += canvas->getHeight() / 2.;
		y -= map_length * tile_size * canvas->scale / canvas->magic * 2.;
		y += canvas->center.second * canvas->scale * tile_size / 2.;

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

		texture->bind(0);

		glEnable(GL_BLEND); CHECKGL
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); CHECKGL
		glBindVertexArray(quadVAO); CHECKGL
		glDrawArrays(GL_TRIANGLES, 0, 6); CHECKGL
		glBindVertexArray(0); CHECKGL
	}

	// TODO: share the bulk of the code with the `const TexturePtr &` override.
	void SingleSpriteRenderer::drawOnMap(GL::Texture &texture, const RenderOptions &options) {
		if (!initialized)
			return;

		auto size_x = options.sizeX;
		auto size_y = options.sizeY;
		auto x = options.x;
		auto y = options.y;

		const GLsizei texture_width = texture.getWidth();
		const GLsizei texture_height = texture.getHeight();

		if (size_x < 0)
			size_x = texture_width;
		if (size_y < 0)
			size_y = texture_height;

		assert(canvas != nullptr);
		RealmPtr realm = canvas->game->getActiveRealm();
		TileProvider &provider = realm->tileProvider;
		TilesetPtr tileset     = provider.getTileset(*canvas->game);
		const auto tile_size   = tileset->getTileSize();
		const auto map_length  = CHUNK_SIZE * REALM_DIAMETER;

		x *= tile_size * canvas->scale / 2.;
		y *= tile_size * canvas->scale / 2.;

		const double x_scale = options.scaleX;
		const double y_scale = options.scaleY;
		auto viewport_x = options.viewportX;
		auto viewport_y = options.viewportY;

		if (viewport_x < 0)
			viewport_x *= -canvas->getWidth();

		if (viewport_y < 0)
			viewport_y *= -canvas->getHeight();

		x += viewport_x / 2.;
		x -= map_length * tile_size * canvas->scale / 4.; // TODO: the math here is a little sus... things might cancel out
		x += canvas->center.first * canvas->scale * tile_size / 2.;

		y += viewport_y / 2.;
		y -= map_length * tile_size * canvas->scale / 4.;
		y += canvas->center.second * canvas->scale * tile_size / 2.;

		shader.bind();

		glm::mat4 model = glm::mat4(1.);
		// first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
		model = glm::translate(model, glm::vec3(x - options.offsetX * canvas->scale * x_scale, y - options.offsetY * canvas->scale * y_scale, 0.));
		if (options.angle != 0) {
			model = glm::translate(model, glm::vec3(.5 * texture_width, .5 * texture_height, 0.)); // move origin of rotation to center of quad
			model = glm::rotate(model, float(glm::radians(options.angle)), glm::vec3(0., 0., 1.)); // then rotate
			model = glm::translate(model, glm::vec3(-.5 * texture_width, -.5 * texture_height, 0.)); // move origin back
		}
		model = glm::scale(model, glm::vec3(texture_width * x_scale * canvas->scale / 2., texture_height * y_scale * canvas->scale / 2., 2.)); // last scale

		shader.set("model", model);
		shader.set("spriteColor", options.color);
		const double multiplier_x = 2. / texture_width;
		const double multiplier_y = 2. / texture_height;
		shader.set("texturePosition", options.offsetX * multiplier_x, options.offsetY * multiplier_y, size_x / texture_width, size_y / texture_width);

		texture.bind(0);

		glEnable(GL_BLEND); CHECKGL
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); CHECKGL
		glBindVertexArray(quadVAO); CHECKGL
		glDrawArrays(GL_TRIANGLES, 0, 6); CHECKGL
		glBindVertexArray(0); CHECKGL
	}

	void SingleSpriteRenderer::reset() {
		shader.init(spriteVert(), spriteFrag());
		initRenderData();
	}

	void SingleSpriteRenderer::initRenderData() {
		if (initialized) {
			glDeleteVertexArrays(1, &quadVAO); CHECKGL
		}

		static const float vertices[] {
			0, 1, 0, 1,
			1, 0, 1, 0,
			0, 0, 0, 0,
			0, 1, 0, 1,
			1, 1, 1, 1,
			1, 0, 1, 0
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

	void SingleSpriteRenderer::drawOnScreen(GL::Texture &texture, const RenderOptions &options_ref) {
		if (!initialized)
			return;

		const auto texture_width  = texture.getWidth();
		const auto texture_height = texture.getHeight();

		RenderOptions options = options_ref;
		options.sizeX = options.sizeX < 0.f? -options.sizeX * texture_width  : options.sizeX;
		options.sizeY = options.sizeY < 0.f? -options.sizeY * texture_height : options.sizeY;
		setupShader(texture_width, texture_height, options);
		allowRepeating(options, texture_width, texture_height);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, options.wrapMode); CHECKGL
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, options.wrapMode); CHECKGL
		texture.bind(0);

		glEnable(GL_BLEND); CHECKGL
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); CHECKGL
		glBindVertexArray(quadVAO); CHECKGL
		glDrawArrays(GL_TRIANGLES, 0, 6); CHECKGL
		glBindVertexArray(0); CHECKGL
	}

	void SingleSpriteRenderer::drawOnScreen(const TexturePtr &texture, const RenderOptions &options_ref) {
		assert(texture);

		if (!initialized)
			return;

		const auto texture_width  = texture->width;
		const auto texture_height = texture->height;

		RenderOptions options = options_ref;
		options.sizeX = options.sizeX < 0.f? -options.sizeX * texture_width  : options.sizeX;
		options.sizeY = options.sizeY < 0.f? -options.sizeY * texture_height : options.sizeY;
		setupShader(texture_width, texture_height, options);
		allowRepeating(options, texture_width, texture_height);

		texture->bind(0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, options.wrapMode); CHECKGL
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, options.wrapMode); CHECKGL

		glEnable(GL_BLEND); CHECKGL
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); CHECKGL
		glBindVertexArray(quadVAO); CHECKGL
		glDrawArrays(GL_TRIANGLES, 0, 6); CHECKGL
		glBindVertexArray(0); CHECKGL
	}

	void SingleSpriteRenderer::allowRepeating(const RenderOptions &options, int texture_width, int texture_height) {
		float x = 1;
		float y = 1;

		if ((options.wrapMode == GL_REPEAT || options.wrapMode == GL_MIRRORED_REPEAT) && (options.sizeX > texture_width || options.sizeY > texture_height)) {
			x = std::max(options.sizeX / texture_width,  1.0);
			y = std::max(options.sizeY / texture_height, 1.0);
		}

		if (x != lastXCoord || y != lastYCoord) {
			lastXCoord = x;
			lastYCoord = y;
			const float vertices[] {
				0, y, 0, y,
				x, 0, x, 0,
				0, 0, 0, 0,
				0, y, 0, y,
				x, y, x, y,
				x, 0, x, 0,
			};
			glBindBuffer(GL_ARRAY_BUFFER, vbo); CHECKGL
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); CHECKGL
		}
	}

	void SingleSpriteRenderer::setupShader(int texture_width, int texture_height, const RenderOptions &options) {
		glm::mat4 model = glm::mat4(1.);
		// first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
		model = glm::translate(model, glm::vec3(options.x - options.offsetX * options.scaleX, options.y - options.offsetY * options.scaleY, 0.f));
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
		const double multiplier = 1.;
		const double multiplier_x = multiplier / texture_width;
		const double multiplier_y = multiplier / texture_height;
		shader.set("texturePosition", options.offsetX * multiplier_x, options.offsetY * multiplier_y, options.sizeX / texture_width, options.sizeY / texture_height);
		shader.set("sprite", 0);
	}
}
