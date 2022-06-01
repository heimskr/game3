#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "resources.h"
#include "ui/TilemapRenderer.h"
#include <GL/glu.h>

// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Renderers/GeometryRenderer.cs

namespace Game3 {
	TilemapRenderer::~TilemapRenderer() {
		glDeleteVertexArrays(1, &vaoHandle);
		glDeleteBuffers(1, &vboHandle);
		glDeleteProgram(shaderHandle);
	}

	void TilemapRenderer::initialize(const std::shared_ptr<Tilemap> &tilemap_) {
		tilemap = tilemap_;
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		createShader();
		generateVertexBufferObject();
		generateVertexArrayObject();
	}

	void TilemapRenderer::render(NVGcontext *context, int font) {
		glUseProgram(shaderHandle);
		glBindTexture(GL_TEXTURE_2D, tilemap->handle);
		glBindVertexArray(vaoHandle);
		glm::mat4 projection(1.f);
		projection = glm::scale(projection, {tilemap->tileSize, -tilemap->tileSize, 1}) *
		             glm::scale(projection, {scale / backBufferWidth, scale / backBufferHeight, 1}) *
		             glm::translate(projection,
		                 {center.x() - tilemap->width / 2.f, center.y() - tilemap->height / 2.f, 0});
		glUseProgram(shaderHandle);
		glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniform2i(glGetUniformLocation(shaderHandle, "mapSize"), tilemap->width, tilemap->height);
		glUniform2i(glGetUniformLocation(shaderHandle, "setSize"), tilemap->setWidth / tilemap->tileSize, tilemap->setHeight / tilemap->tileSize);
		glDrawArrays(GL_POINTS, 0, tilemap->tiles.size());
		if (context != nullptr && font != -1) {
			constexpr float font_size = 12.f;
			nvgFontSize(context, font_size);
			nvgFillColor(context, {1.f, 0.f, 0.f, 1.f});
			nvgFontFaceId(context, font);
			auto draw_text = [&](int x, int y, std::string_view text) {
				float tx = center.x() * 64.f + backBufferWidth  / 2.f - (tilemap->width) * tilemap->tileSize + scale + x * tilemap->tileSize * scale / 2.f;
				float ty = center.y() * 64.f + backBufferHeight / 2.f - (tilemap->height - 1) * tilemap->tileSize + font_size + scale + y * tilemap->tileSize * scale / 2.f;
				nvgText(context, tx, ty, text.data(), nullptr);
			};

			for (int y = 0; y < tilemap->height; ++y) {
				for (int x = 0; x < tilemap->width; ++x) {
					// std::cerr << x << ' ';
					const int sum = tilemap->sums.at(x + y * tilemap->width);
					const int id = (*tilemap)(x, y);
					draw_text(x, y, std::to_string(sum) + ":" + std::to_string(id));
				}
				// std::cerr << '\n';
			}
		}
	}

	void TilemapRenderer::onBackBufferResized(int width, int height) {
		if (width == backBufferWidth && height == backBufferHeight)
			return;
		backBufferWidth = width;
		backBufferHeight = height;
	}

	static void check(int handle, bool is_link = false) {
		int success;
		char info[1024];
		if (is_link)
			glGetProgramiv(handle, GL_LINK_STATUS, &success);
		else
			glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLsizei len = 666;
			if (is_link)
				glGetProgramInfoLog(handle, GL_INFO_LOG_LENGTH, &len, info);
			else
				glGetShaderInfoLog(handle, 1024, &len, info);
			std::cerr << "Error with " << handle << " (l=" << len << "): " << info << '\n';
		}
	}

	void TilemapRenderer::createShader() {
		const GLchar *vert_ptr = reinterpret_cast<const GLchar *>(tilemap_vert);
		int vert_handle = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vert_handle, 1, &vert_ptr, reinterpret_cast<const GLint *>(&tilemap_vert_len));
		glCompileShader(vert_handle);
		check(vert_handle);

		const GLchar *geom_ptr = reinterpret_cast<const GLchar *>(tilemap_geom);
		int geom_handle = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geom_handle, 1, &geom_ptr, reinterpret_cast<const GLint *>(&tilemap_geom_len));
		glCompileShader(geom_handle);
		check(geom_handle);

		const GLchar *frag_ptr = reinterpret_cast<const GLchar *>(tilemap_frag);
		int frag_handle = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(frag_handle, 1, &frag_ptr, reinterpret_cast<const GLint *>(&tilemap_frag_len));
		glCompileShader(frag_handle);
		check(frag_handle);

		shaderHandle = glCreateProgram();
		glAttachShader(shaderHandle, vert_handle);
		glAttachShader(shaderHandle, geom_handle);
		glAttachShader(shaderHandle, frag_handle);
		glLinkProgram(shaderHandle);
		check(shaderHandle, true);

		glDetachShader(shaderHandle, vert_handle);
		glDeleteShader(vert_handle);

		glDetachShader(shaderHandle, geom_handle);
		glDeleteShader(geom_handle);

		glDetachShader(shaderHandle, frag_handle);
		glDeleteShader(frag_handle);
	}

	void TilemapRenderer::generateVertexBufferObject() {
		glGenBuffers(1, &vboHandle);
		glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
		glBufferData(GL_ARRAY_BUFFER, tilemap->tiles.size() * sizeof(decltype(tilemap->tiles)::value_type),
			tilemap->tiles.data(), GL_STATIC_DRAW);
	}

	void TilemapRenderer::generateVertexArrayObject() {
		glGenVertexArrays(1, &vaoHandle);
		glBindVertexArray(vaoHandle);
		glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
		glEnableVertexAttribArray(0);
		glVertexAttribIPointer(0, 1, GL_UNSIGNED_BYTE, sizeof(char), nullptr);
	}
}
