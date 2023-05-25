// Credit: https://learnopengl.com/In-Practice/Text-Rendering

#include <atomic>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#include "Log.h"
#include "Tileset.h"
#include "game/ClientGame.h"
#include "realm/Realm.h"
#include "ui/Canvas.h"
#include "ui/TextRenderer.h"

namespace Game3 {
	TextRenderer::TextRenderer(Canvas &canvas_): canvas(&canvas_), shader("TextRenderer") {
		shader.init(text_vert, text_frag);
		initRenderData();
	}

	TextRenderer::~TextRenderer() {
		remove();
	}

	void TextRenderer::remove() {
		if (initialized) {
			glDeleteVertexArrays(1, &vao);
			glDeleteBuffers(1, &vbo);
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
		FT_Set_Pixel_Sizes(face, 0, 48);
		characters.clear();
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1); CHECKGL

		for (gunichar ch = 32; ch < 127; ++ch) {
			// Load character glyph
			if (FT_Load_Char(face, ch, FT_LOAD_RENDER))
				throw std::runtime_error("Failed to load glyph " + std::to_string(static_cast<uint32_t>(ch)));

			GLuint texture;
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
		SUCCESS("TextRenderer::initRenderData() finished.");
	}

	void TextRenderer::update(int backbuffer_width, int backbuffer_height) {
		if (backbuffer_width != backbufferWidth || backbuffer_height != backbufferHeight) {
			backbufferWidth = backbuffer_width;
			backbufferHeight = backbuffer_height;
			glm::mat4 projection = glm::ortho(0.f, static_cast<float>(backbuffer_width), static_cast<float>(backbuffer_height), 0.f, -1.f, 1.f);
			shader.bind();
			shader.set("projection", projection);
		}
	}

	void TextRenderer::drawOnMap(std::string_view text, float x, float y, TextAlign align, float scale, float angle, float alpha) {
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

	void TextRenderer::drawOnMap(std::string_view text, const TextRenderOptions &options) {
		if (!initialized)
			return;

		auto &provider = canvas->game->activeRealm->tileProvider;
		const auto &tileset   = provider.getTileset(*canvas->game);
		const auto tile_size  = tileset->getTileSize();
		const auto map_length = CHUNK_SIZE * REALM_DIAMETER;

		auto x = options.x;
		auto y = options.y;
		INFO(x << " :: " << y);
		auto scale_x = options.scaleX / 2.f;
		auto scale_y = options.scaleY / 2.f;

		x *= tile_size * canvas->scale / 2.f;
		y *= tile_size * canvas->scale / 2.f;

		x += canvas->width() / 2.f;
		x -= map_length * tile_size * canvas->scale / canvas->magic * 2.f; // TODO: the math here is a little sus... things might cancel out
		x += centerX * canvas->scale * tile_size / 2.f;

		y += canvas->height() / 2.f;
		y -= map_length * tile_size * canvas->scale / canvas->magic * 2.f;
		y += centerY * canvas->scale * tile_size / 2.f;

		shader.bind();

		const auto text_width = textWidth(text, 1.f);
		const auto text_height = textHeight(text, 1.f);

		glm::mat4 model = glm::mat4(1.f);
		// first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
		model = glm::translate(model, glm::vec3(x, y, 0.f));
		model = glm::translate(model, glm::vec3(0.5f * text_width, 0.5f * text_height, 0.f)); // move origin of rotation to center of quad
		model = glm::rotate(model, glm::radians(options.angle), glm::vec3(0.f, 0.f, 1.f)); // then rotate
		model = glm::translate(model, glm::vec3(-0.5f * text_width, -0.5f * text_height, 0.f)); // move origin back
		model = glm::scale(model, glm::vec3(text_width * scale_x * canvas->scale / 2.f, -text_height * scale_y * canvas->scale / 2.f, 2.f)); // last scale

		shader.set("model", model);
		shader.set("textColor", options.color.red, options.color.green, options.color.blue, options.color.alpha);

		glActiveTexture(GL_TEXTURE0); CHECKGL
		glBindVertexArray(vao); CHECKGL

		for (const char ch: text) {
			const auto &character = characters.at(ch);

			const float xpos = x + character.bearing.x * scale_x;
			const float ypos = y - (character.size.y - character.bearing.y) * scale_y;
			const float w = character.size.x * scale_x;
			const float h = character.size.y * scale_y;

			// Update VBO for each character
			const float vertices[6][4] = {
				{ xpos,     ypos + h,   0.0f, 0.0f },
				{ xpos,     ypos,       0.0f, 1.0f },
				{ xpos + w, ypos,       1.0f, 1.0f },

				{ xpos,     ypos + h,   0.0f, 0.0f },
				{ xpos + w, ypos,       1.0f, 1.0f },
				{ xpos + w, ypos + h,   1.0f, 0.0f }
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
			x += (character.advance >> 6) * options.scaleX; // Bitshift by 6 to get value in pixels (2^6 = 64)
		}

		glBindVertexArray(0); CHECKGL
		glBindTexture(GL_TEXTURE_2D, 0); CHECKGL
	}

	float TextRenderer::textWidth(std::string_view text, float scale) {
		float out = 0.f;
		for (const char ch: text)
			out += scale * (characters.at(ch).advance >> 6);
		return out;
	}

	float TextRenderer::textHeight(std::string_view text, float scale) {
		float out = 0.f;
		for (const char ch: text)
			out = std::max(out, characters.at(ch).size.y * scale);
		return out;
	}

	// void drawText(Canvas &canvas, const char *text) {
	// 	initText();

	// 	glEnable(GL_BLEND); CHECKGL
	// 	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); CHECKGL

	// 	const float screen_width = canvas.width();
	// 	const float screen_height = canvas.height();

	// 	glm::mat4 projection = glm::ortho(0.0f, screen_width, 0.0f, screen_height);

	// }
}
