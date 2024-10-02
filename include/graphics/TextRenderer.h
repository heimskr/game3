#pragma once

// Credit: https://learnopengl.com/In-Practice/Text-Rendering

#include "graphics/Color.h"
#include "graphics/GL.h"
#include "graphics/HasBackbuffer.h"
#include "graphics/Shader.h"
#include "types/Position.h"
#include "types/Types.h"

#include <memory>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Game3 {
	class UString;
	class Window;

	enum class TextAlign {Left, Center, Right};

	struct TextRenderOptions {
		double x      = 0;
		double y      = 0;
		double scaleX = 1;
		double scaleY = 1;
		double angle  = 0;
		double wrapWidth = 0;
		Color color{1, 1, 1, 1};
		TextAlign align = TextAlign::Left;
		bool alignTop = false;
		Color shadow{0, 0, 0, 1};
		Vector2f shadowOffset{.025f, .025f};
		float *heightOut = nullptr;
		bool ignoreNewline = false;
	};

	class TextRenderer: public HasBackbuffer {
		public:
			Window *window = nullptr;
			Shader shader;
			double centerX = 0;
			double centerY = 0;
			uint32_t fontScale = 48;

			struct Character {
				GLuint     textureID; // ID handle of the glyph texture
				glm::ivec2 size;      // Size of glyph
				glm::ivec2 bearing;   // Offset from baseline to left/top of glyph
				FT_Pos     advance;   // Offset to advance to next glyph
			};

			std::unordered_map<uint32_t, Character> characters;

			TextRenderer(Window &, uint32_t font_scale = 96);
			~TextRenderer();

			void remove();
			void update(const Window &) override;
			void update(int width, int height) override;

			void drawOnMap(const UString &text, float x, float y, TextAlign align, float scale, float angle, float alpha);
			void drawOnMap(const UString &text, TextRenderOptions = {});
			void drawOnScreen(const UString &text, TextRenderOptions = {});
			void operator()(const UString &text, const TextRenderOptions & = {});
			float textWidth(uint32_t character, float scale = 1.f) const;
			float textWidth(const UString &text, float scale = 1.f) const;
			float textHeight(const UString &text, float scale = 1.f) const;
			float textHeight(const UString &text, float scale, float wrap_width) const;
			float getIHeight() const;

			void reset();
			void initRenderData();

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
			mutable std::optional<float> cachedIHeight;

			glm::mat4 projection;

			std::unique_ptr<FT_Library, FreeLibrary> freetypeLibrary;
			std::unique_ptr<FT_Face, FreeFace> freetypeFace;

			const Character & getCharacter(uint32_t) const;
			const Character & getCharacter(uint32_t);
	};
}
