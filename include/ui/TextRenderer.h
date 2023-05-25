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
		Color color {};
		TextAlign align = TextAlign::Left;
	};

	class TextRenderer {
		public:
			Canvas *canvas = nullptr;
			Shader shader;
			float divisor = 1.f;
			float centerX = 0.f;
			float centerY = 0.f;

			TextRenderer(Canvas &);
			~TextRenderer();

			// TextRenderer & operator=(TextRenderer &&);

			void remove();
			void update(int backbuffer_width, int backbuffer_height);

			void drawOnMap(std::string_view text, float x, float y, TextAlign align, float scale, float angle, float alpha);
			void drawOnMap(std::string_view text, const TextRenderOptions & = {});
			void drawOnScreen(std::string_view text, const TextRenderOptions & = {});
			float textWidth(std::string_view, float scale = 1.f);
			float textHeight(std::string_view, float scale = 1.f);

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

			struct Character {
				GLuint     textureID; // ID handle of the glyph texture
				glm::ivec2 size;      // Size of glyph
				glm::ivec2 bearing;   // Offset from baseline to left/top of glyph
				FT_Pos     advance;   // Offset to advance to next glyph
			};

			GLuint vao = 0;
			GLuint vbo = 0;
			bool initialized = false;
			int backbufferWidth = -1;
			int backbufferHeight = -1;

			glm::mat4 projection;
			glm::vec4 v4;

			std::unique_ptr<FT_Library, FreeLibrary> freetypeLibrary;
			std::unique_ptr<FT_Face, FreeFace> freetypeFace;
			std::unordered_map<gunichar, Character> characters;

			void initRenderData();
			void setupShader(std::string_view, const TextRenderOptions &);
			void hackY(float &y, float y_offset, float scale);
	};
}
