#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GL_GLEXT_PROTOTYPES
#include <GL/glew.h>

#include "resources.h"
#include "ui/GeometryRenderer.h"
#include <GL/glu.h>

// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Renderers/GeometryRenderer.cs

namespace Game3 {
	GeometryRenderer::~GeometryRenderer() {
		glDeleteVertexArrays(1, &vaoHandle);
		glDeleteBuffers(1, &vboHandle);
		glDeleteProgram(shaderHandle);
	}

	void GeometryRenderer::initialize(const std::shared_ptr<Tilemap> &tilemap_) {
		tilemap = tilemap_;
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		createShader();
		generateVertexBufferObject();
		generateVertexArrayObject();
	}

	void GeometryRenderer::render() {
		glUseProgram(shaderHandle);
		glBindTexture(GL_TEXTURE_2D, tilemap->texture.id);
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
	}

	void GeometryRenderer::createShader() {
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

	void GeometryRenderer::generateVertexBufferObject() {
		glGenBuffers(1, &vboHandle);
		glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
		glBufferData(GL_ARRAY_BUFFER, tilemap->tiles.size() * sizeof(decltype(tilemap->tiles)::value_type),
			tilemap->tiles.data(), GL_STATIC_DRAW);
	}

	void GeometryRenderer::generateVertexArrayObject() {
		glGenVertexArrays(1, &vaoHandle);
		glBindVertexArray(vaoHandle);
		glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
		glEnableVertexAttribArray(0);
		glVertexAttribIPointer(0, 1, GL_UNSIGNED_BYTE, sizeof(char), nullptr);
	}
}
