#pragma once

// Credit: https://learnopengl.com/In-Practice/Text-Rendering

#include <memory>

#include "util/GL.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <gtkmm.h>

#include "Shader.h"
#include "Types.h"

namespace Game3 {
	class Canvas;

	enum class TextAlign {Left, Center, Right};

	struct TextRenderOptions {
		float x      = 0.f;
		float y      = 0.f;
		float scaleX = 1.f;
		float scaleY = 1.f;
		float angle  = 0.f;
		Color color {0.f, 0.f, 0.f, 1.f};
		TextAlign align = TextAlign::Left;
	};

	class TextRenderer {
		public:
			Canvas *canvas = nullptr;
			Shader shader;
			float divisor = 1.f;
			float centerX = 0.f;
			float centerY = 0.f;
			uint32_t fontScale = 48.f;

			struct Character {
				GLuint     textureID; // ID handle of the glyph texture
				glm::ivec2 size;      // Size of glyph
				glm::ivec2 bearing;   // Offset from baseline to left/top of glyph
				FT_Pos     advance;   // Offset to advance to next glyph
			};

			std::unordered_map<gunichar, Character> characters;

			TextRenderer(Canvas &, uint32_t font_scale = 96);
			~TextRenderer();

			// TextRenderer & operator=(TextRenderer &&);

			void remove();
			void update(int backbuffer_width, int backbuffer_height);

			void drawOnMap(Glib::ustring text, float x, float y, TextAlign align, float scale, float angle, float alpha);
			void drawOnMap(Glib::ustring text, TextRenderOptions = {});
			void drawOnScreen(Glib::ustring text, const TextRenderOptions & = {});
			float textWidth(Glib::ustring, float scale = 1.f);
			float textHeight(Glib::ustring, float scale = 1.f);

			template <typename... Args>
			void operator()(Args &&...args) {
				drawOnMap(std::forward<Args>(args)...);
			}

			void reset();

		private:
			struct FreeLibrary {
				void operator()(FT_Library *library) const {
					FT_Done_FreeType(*library);
					delete library;
				}
			};

			struct FreeFace {
				void operator()(FT_Face *face) const {
					FT_Done_Face(*face);
					delete face;
				}
			};

			GLuint vao = 0;
			GLuint vbo = 0;
			bool initialized = false;
			int backbufferWidth = -1;
			int backbufferHeight = -1;

			glm::mat4 projection;

			std::unique_ptr<FT_Library, FreeLibrary> freetypeLibrary;
			std::unique_ptr<FT_Face, FreeFace> freetypeFace;

			void initRenderData();
			const Character & getCharacter(gunichar) const;
	};
}
