#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ui/TilemapRenderer.h"

// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Renderers/GeometryRenderer.cs

namespace Game3 {
	void TilemapRenderer::initialize(const std::shared_ptr<Tilemap> &tilemap_) {
		tilemap = tilemap_;
		glClipControl(GL_UPPER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		createShader();
		generateVertexBufferObject();
		generateVertexArrayObject();
	}

	void TilemapRenderer::render() {
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

	
}
