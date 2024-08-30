// Credit: https://learnopengl.com/In-Practice/Text-Rendering

#include <atomic>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#include "Log.h"
#include "graphics/Tileset.h"
#include "game/ClientGame.h"
#include "graphics/TextRenderer.h"
#include "realm/Realm.h"
#include "ui/Canvas.h"
#include "util/FS.h"

namespace Game3 {
	constexpr float LINE_HEIGHT = 1.5;
}

namespace Game3 {
	namespace {
		const std::string & textFrag() { static auto out = readFile("resources/text.frag"); return out; }
		const std::string & textVert() { static auto out = readFile("resources/text.vert"); return out; }
	}

	TextRenderer::TextRenderer(Canvas &canvas_, uint32_t font_scale): canvas(&canvas_), shader("TextRenderer"), fontScale(font_scale) {
		shader.init(textVert(), textFrag());
	}

	TextRenderer::~TextRenderer() {
		remove();
	}

	void TextRenderer::remove() {
		if (initialized) {
			glDeleteVertexArrays(1, &vao); CHECKGL
			glDeleteBuffers(1, &vbo); CHECKGL
			vao = 0;
			vbo = 0;
			initialized = false;
		}
	}

	void TextRenderer::initRenderData() {
		if (initialized)
			return;

		auto freetypeLibrary = std::unique_ptr<FT_Library, FreeLibrary>(new FT_Library);
		if (FT_Init_FreeType(freetypeLibrary.get()))
			throw std::runtime_error("Couldn't initialize FreeType");

		auto freetypeFace = std::unique_ptr<FT_Face, FreeFace>(new FT_Face);
		if (FT_New_Face(*freetypeLibrary, "resources/CozetteVector.ttf", 0, freetypeFace.get()))
			throw std::runtime_error("Couldn't initialize font");

		auto &face = *freetypeFace;
		FT_Set_Pixel_Sizes(face, 0, fontScale);
		characters.clear();
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1); CHECKGL

		for (gunichar ch = 32; ch < 127; ++ch) {
			// Load character glyph
			if (FT_Load_Char(face, ch, FT_LOAD_RENDER))
				throw std::runtime_error("Failed to load glyph " + std::to_string(static_cast<uint32_t>(ch)));

			GLuint texture = 0;
			glGenTextures(1, &texture); CHECKGL
			glBindTexture(GL_TEXTURE_2D, texture); CHECKGL
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer); CHECKGL

			// Set texture options
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); CHECKGL
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); CHECKGL
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); CHECKGL
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); CHECKGL

			// Store character for later use
			characters.emplace(ch, Character {
				texture,
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				face->glyph->advance.x
			});
		}

		glGenVertexArrays(1, &vao); CHECKGL
		glGenBuffers(1, &vbo); CHECKGL
		glBindVertexArray(vao); CHECKGL
		glBindBuffer(GL_ARRAY_BUFFER, vbo); CHECKGL
		glBufferData(GL_ARRAY_BUFFER, 4 * 6 * sizeof(float), nullptr, GL_DYNAMIC_DRAW); CHECKGL
		glEnableVertexAttribArray(0); CHECKGL
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0); CHECKGL
		glBindBuffer(GL_ARRAY_BUFFER, 0); CHECKGL
		glBindVertexArray(0); CHECKGL

		initialized = true;
	}

	void TextRenderer::update(const Canvas &canvas) {
		centerX = canvas.center.first;
		centerY = canvas.center.second;
		update(canvas.getWidth(), canvas.getHeight());
	}

	void TextRenderer::update(int width, int height) {
		if (width != backbufferWidth || height != backbufferHeight) {
			HasBackbuffer::update(width, height);
			projection = glm::ortho(0.f, static_cast<float>(width), 0.f, static_cast<float>(height), -1.f, 1.f);
			shader.bind();
			shader.set("projection", projection);
		}
	}

	void TextRenderer::drawOnMap(const Glib::ustring &text, float x, float y, TextAlign align, float scale, float angle, float alpha) {
		drawOnMap(text, TextRenderOptions {
			.x = x,
			.y = y,
			.scaleX = scale,
			.scaleY = scale,
			.angle = angle,
			.color = {0.f, 0.f, 0.f, alpha},
			.align = align,
		});
	}

	void TextRenderer::drawOnMap(const Glib::ustring &text, TextRenderOptions options) {
		if (!initialized)
			initRenderData();

		if (0 < options.shadow.alpha) {
			Color color = options.shadow;
			Color shadow{0, 0, 0, 0};
			double x = options.x + options.shadowOffset.x * options.scaleX;
			double y = options.y + options.shadowOffset.y * options.scaleY;
			std::swap(options.color, color);
			std::swap(options.shadow, shadow);
			std::swap(options.x, x);
			std::swap(options.y, y);
			drawOnMap(text, options);
			std::swap(options.color, color);
			std::swap(options.shadow, shadow);
			std::swap(options.x, x);
			std::swap(options.y, y);
		}

		RealmPtr realm = canvas->game->getActiveRealm();
		TileProvider &provider = realm->tileProvider;
		TilesetPtr tileset     = provider.getTileset(*canvas->game);
		const auto tile_size   = tileset->getTileSize();
		const auto map_length  = CHUNK_SIZE * REALM_DIAMETER;

		auto &x = options.x;
		auto &y = options.y;
		auto &scale_x = options.scaleX;
		auto &scale_y = options.scaleY;

		scale_x *= canvas->scale * 6 / fontScale;
		scale_y *= canvas->scale * 6 / fontScale;

		x *= 8;
		y *= -8;

		x += backbufferWidth / 2 / canvas->scale;
		y += backbufferHeight / 2 / canvas->scale;

		x -= map_length * tile_size / 4;
		y += map_length * tile_size / 4;

		x += centerX * 8;
		y -= centerY * 8;

		x *= canvas->scale;
		y *= canvas->scale;

		if (options.align == TextAlign::Center)
			x -= textWidth(text, scale_x) / 2;
		else if (options.align == TextAlign::Right)
			x -= textWidth(text, scale_x);

		shader.bind();
		shader.set("textColor", options.color.red, options.color.green, options.color.blue, options.color.alpha); CHECKGL

		glActiveTexture(GL_TEXTURE0); CHECKGL
		glBindVertexArray(vao); CHECKGL

		for (const auto ch: text) {
			const auto &character = getCharacter(ch);

			const float xpos = x + character.bearing.x * scale_x;
			const float ypos = y - (character.size.y - character.bearing.y) * scale_y;
			const float w = character.size.x * scale_x;
			const float h = character.size.y * scale_y;

			// Update VBO for each character
			const float vertices[6][4] = {
				{xpos,     ypos + h, 0.0f, 0.0f},
				{xpos,     ypos,     0.0f, 1.0f},
				{xpos + w, ypos,     1.0f, 1.0f},

				{xpos,     ypos + h, 0.0f, 0.0f},
				{xpos + w, ypos,     1.0f, 1.0f},
				{xpos + w, ypos + h, 1.0f, 0.0f}
			};

			// Render glyph texture over quad
			glBindTexture(GL_TEXTURE_2D, character.textureID); CHECKGL
			// Update content of VBO memory
			glBindBuffer(GL_ARRAY_BUFFER, vbo); CHECKGL
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); CHECKGL
			glBindBuffer(GL_ARRAY_BUFFER, 0); CHECKGL
			// Render quad
			glDrawArrays(GL_TRIANGLES, 0, 6); CHECKGL
			// Advance cursors for next glyph (note that advance is number of 1/64 pixels)
			x += (character.advance >> 6) * scale_x; // Bitshift by 6 to get value in pixels (2^6 = 64)
		}

		glBindVertexArray(0); CHECKGL
		glBindTexture(GL_TEXTURE_2D, 0); CHECKGL
	}

	void TextRenderer::drawOnScreen(const Glib::ustring &text, TextRenderOptions options) {
		if (!initialized)
			initRenderData();

		if (0 < options.shadow.alpha) {
			Color color = options.shadow;
			Color shadow{0, 0, 0, 0};
			double x = options.x + options.shadowOffset.x * options.scaleX;
			double y = options.y + options.shadowOffset.y * options.scaleY;
			std::swap(options.color, color);
			std::swap(options.shadow, shadow);
			std::swap(options.x, x);
			std::swap(options.y, y);
			drawOnScreen(text, options);
			std::swap(options.color, color);
			std::swap(options.shadow, shadow);
			std::swap(options.x, x);
			std::swap(options.y, y);
		}

		auto &x = options.x;
		auto &y = options.y;
		auto &scale_x = options.scaleX;
		auto &scale_y = options.scaleY;

		const auto i_height = getCharacter('I').size.y * scale_y;
		float y_addition = 0;

		if (options.alignTop) {
			y_addition = i_height;
			y += y_addition;
		}

		y = backbufferHeight - y;

		if (options.align == TextAlign::Center)
			x -= textWidth(text, scale_x) / 2;
		else if (options.align == TextAlign::Right)
			x -= textWidth(text, scale_x);

		shader.bind();
		shader.set("textColor", options.color.red, options.color.green, options.color.blue, options.color.alpha); CHECKGL

		glActiveTexture(GL_TEXTURE0); CHECKGL
		glBindVertexArray(vao); CHECKGL

		const auto wrap_width = options.wrapWidth;
		const auto x_start = x;
		const auto y_start = y;

		auto next_line = [&] {
			x = x_start;
			y -= i_height * LINE_HEIGHT;
		};

		float highest_on_first_line = 0;

		for (const gunichar ch: text) {
			if (!options.ignoreNewline && ch == '\n') {
				next_line();
				continue;
			}

			const Character &character = getCharacter(ch);

			const float w = character.size.x * scale_x;
			const float h = character.size.y * scale_y;
			float xpos = x + character.bearing.x * scale_x;
			float ypos = y - h + character.bearing.y * scale_y;

			if (y == y_start && h > highest_on_first_line) {
				highest_on_first_line = h;
			}

			if (wrap_width > 0 && wrap_width < xpos - x_start + w) {
				next_line();
				xpos = x + character.bearing.x * scale_x;
				ypos = y - h + character.bearing.y * scale_y;
			}

			// Update VBO for each character
			const float vertices[6][4] = {
				{xpos,     ypos + h, 0.0f, 0.0f},
				{xpos,     ypos,     0.0f, 1.0f},
				{xpos + w, ypos,     1.0f, 1.0f},

				{xpos,     ypos + h, 0.0f, 0.0f},
				{xpos + w, ypos,     1.0f, 1.0f},
				{xpos + w, ypos + h, 1.0f, 0.0f}
			};

			// Render glyph texture over quad
			glBindTexture(GL_TEXTURE_2D, character.textureID); CHECKGL
			// Update content of VBO memory
			glBindBuffer(GL_ARRAY_BUFFER, vbo); CHECKGL
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); CHECKGL
			glBindBuffer(GL_ARRAY_BUFFER, 0); CHECKGL
			// Render quad
			glDrawArrays(GL_TRIANGLES, 0, 6); CHECKGL
			// Advance cursor for next glyph (note that advance is number of 1/64 pixels)
			x += (character.advance >> 6) * scale_x; // Bitshift by 6 to get value in pixels (2^6 = 64)
		}

		glBindVertexArray(0); CHECKGL
		glBindTexture(GL_TEXTURE_2D, 0); CHECKGL

		if (options.heightOut) {
			*options.heightOut = y_start - y + highest_on_first_line;
		}
	}

	void TextRenderer::operator()(const Glib::ustring &text, const TextRenderOptions &options) {
		drawOnScreen(text, options);
	}

	float TextRenderer::textWidth(gunichar character, float scale) const {
		return scale * (getCharacter(character).advance >> 6);
	}

	float TextRenderer::textWidth(const Glib::ustring &text, float scale) const {
		float out = 0.f;
		for (const char ch: text)
			out += scale * (getCharacter(ch).advance >> 6);
		return out;
	}

	float TextRenderer::textHeight(const Glib::ustring &text, float scale) const {
		float out = 0.f;
		for (const char ch: text)
			out = std::max(out, getCharacter(ch).size.y * scale);
		return out;
	}

	float TextRenderer::textHeight(const Glib::ustring &text, float scale, float wrap_width) const {
		const auto i_height = getCharacter('I').size.y * scale;
		float x = 0;
		float y = 0;

		auto next_line = [&] {
			x = 0;
			y -= i_height * LINE_HEIGHT;
		};

		float highest_on_first_line = 0;

		for (const gunichar ch: text) {
			if (ch == '\n') {
				next_line();
				continue;
			}

			const Character &character = getCharacter(ch);

			const float w = character.size.x * scale;
			const float h = character.size.y * scale;

			if (y == 0 && h > highest_on_first_line)
				highest_on_first_line = h;

			if (wrap_width > 0 && wrap_width < x + character.bearing.x * scale + w)
				next_line();

			x += (character.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
		}

		return -y + highest_on_first_line;
	}

	float TextRenderer::getIHeight(float scale) const {
		return getCharacter('I').size.y * scale;
	}

	const TextRenderer::Character & TextRenderer::getCharacter(gunichar ch) const {
		if (auto iter = characters.find(ch); iter != characters.end())
			return iter->second;
		return characters.at('?');
	}
}
