#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "resources.h"
#include "ui/TilemapRenderer.h"

// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Renderers/GeometryRenderer.cs

namespace Game3 {
	TilemapRenderer::~TilemapRenderer() {
		glDeleteVertexArrays(1, &vaoHandle);
		glDeleteBuffers(1, &vboHandle);
		glDeleteProgram(shaderHandle);
	}

	void TilemapRenderer::initialize(const std::shared_ptr<Tilemap> &tilemap_) {
		tilemap = tilemap_;
		// glClipControl(GL_UPPER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		createShader();
		generateVertexBufferObject();
		generateVertexArrayObject();
	}

	void TilemapRenderer::render() {
		glClearColor(0.5f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_2D, tilemap->handle);
		glBindVertexArray(vaoHandle);
		glm::mat4 projection(1.f);
		projection = glm::translate(projection, {-center.x(), -center.y(), 0}) *
					 glm::scale(projection, {tilemap->tileSize, tilemap->tileSize, 1}) *
					 glm::scale(projection, {2.f / backBufferWidth, 2.f / backBufferHeight, 1});
		glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "projection"), 1, false, glm::value_ptr(projection));
		glUniform2f(glGetUniformLocation(shaderHandle, "mapSize"), tilemap->width, tilemap->height);
		glUseProgram(shaderHandle);
		glDrawArrays(GL_POINTS, 0, tilemap->tiles.size());
	}

	void TilemapRenderer::onBackBufferResized(int width, int height) {
		if (width == backBufferWidth && height == backBufferHeight)
			return;
		backBufferWidth = width;
		backBufferHeight = height;
		// TODO: is this correct? Is this already handled by nanogui?
		glViewport(0, 0, width, height);
	}

	void TilemapRenderer::createShader() {
		int vert_handle = glCreateShader(GL_VERTEX_SHADER);

		const GLchar *vert_ptr = reinterpret_cast<const GLchar *>(tilemap_vert);
		glShaderSource(vert_handle, 1, &vert_ptr, reinterpret_cast<const GLint *>(&tilemap_vert_len));
		glCompileShader(vert_handle);

		const GLchar *geom_ptr = reinterpret_cast<const GLchar *>(tilemap_geom);
		int geom_handle = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geom_handle, 1, &geom_ptr, reinterpret_cast<const GLint *>(&tilemap_geom_len));
		glCompileShader(geom_handle);

		const GLchar *frag_ptr = reinterpret_cast<const GLchar *>(tilemap_frag);
		int frag_handle = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(frag_handle, 1, &frag_ptr, reinterpret_cast<const GLint *>(&tilemap_frag_len));
		glCompileShader(frag_handle);

		shaderHandle = glCreateProgram();
		glAttachShader(shaderHandle, vert_handle);
		glAttachShader(shaderHandle, geom_handle);
		glAttachShader(shaderHandle, frag_handle);
		glLinkProgram(shaderHandle);

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
