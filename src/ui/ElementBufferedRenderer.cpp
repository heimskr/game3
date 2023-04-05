#include <iostream>

#include "Shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "resources.h"
#include "Tiles.h"
#include "ui/ElementBufferedRenderer.h"
#include "util/Util.h"

// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Renderers/ElementBufferedRenderer.cs

namespace Game3 {
	ElementBufferedRenderer::~ElementBufferedRenderer() {
		reset();
	}

	void ElementBufferedRenderer::reset() {
		if (initialized) {
			glDeleteVertexArrays(1, &vaoHandle);
			glDeleteBuffers(1, &eboHandle);
			glDeleteBuffers(1, &vboHandle);
			glDeleteProgram(shaderHandle);
			glDeleteTextures(1, &lfbTexture);
			glDeleteSamplers(1, &sampler);
			tilemap.reset();
			initialized = false;
		}
	}

	void ElementBufferedRenderer::init(const TilemapPtr &tilemap_, const TileSet &tileset) {
		if (initialized)
			reset();
		tilemap = tilemap_;
		createShader();
		generateVertexBufferObject();
		generateElementBufferObject();
		generateVertexArrayObject();
		generateLightingFrameBuffer();
		generateSampler();
		const auto bright_shorts = tileset.getBright();
		brightTiles.assign(bright_shorts.begin(), bright_shorts.end());
		brightTiles.resize(8, -1);
		initialized = true;
	}

	void ElementBufferedRenderer::render(float divisor) {
		if (!initialized)
			return;
		tilemap->texture.bind();


		CHECKGL


		std::cout << lfbHandle << " ~ " << lfbTexture << std::endl;
		GLint gtk_buffer = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &gtk_buffer); CHECKGL
		glBindFramebuffer(GL_FRAMEBUFFER, lfbHandle); CHECKGL
		// GLenum bufs[] {GL_NONE};
		// glDrawBuffers(1, bufs);
		glDrawBuffer(GL_COLOR_ATTACHMENT0); CHECKGL
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lfbTexture, 0); CHECKGL
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); CHECKGL
		glClearColor(0.0f, 1.0f, 0.0f, 1.0f); CHECKGL
		// glDisable(GL_DEPTH_TEST); CHECKGL
		// glEnable(GL_DEPTH_TEST); CHECKGL
		// glColor4f(1.f, 0.f, 0.f, 1.f); CHECKGL
		// glRecti(100, 100, 200, 200); CHECKGL

		rectangle.drawOnScreen({1.f, 0.f, 0.f, 1.f}, 10.f, 10.f, 20.f, 10.f); CHECKGL


		glBindFramebuffer(GL_FRAMEBUFFER, gtk_buffer); CHECKGL
		// glDrawBuffer(GL_BACK); CHECKGL

		glActiveTexture(GL_TEXTURE1); CHECKGL
		glBindTexture(GL_TEXTURE_2D, lfbTexture); CHECKGL

		glBindVertexArray(vaoHandle);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboHandle);
		glm::mat4 projection(1.f);
		projection = glm::scale(projection, {tilemap->tileSize, -tilemap->tileSize, 1}) *
		             glm::scale(projection, {scale / backbufferWidth, scale / backbufferHeight, 1}) *
		             glm::translate(projection, {center.x() - tilemap->width / 2.f, center.y() - tilemap->height / 2.f, 0});
		glUseProgram(shaderHandle);





		glUniform1i(glGetUniformLocation(shaderHandle, "texture0"), 0);
		glUniform1i(glGetUniformLocation(shaderHandle, "texture1"), 1);
		glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniform1f(glGetUniformLocation(shaderHandle,  "divisor"), divisor);
		glUniform1iv(glGetUniformLocation(shaderHandle, "bright_tiles"), brightTiles.size(), brightTiles.data());
		glUniform1i(glGetUniformLocation(shaderHandle,  "tile_size"), static_cast<GLint>(tilemap->tileSize));
		glUniform1i(glGetUniformLocation(shaderHandle,  "tileset_width"), static_cast<GLint>(tilemap->setWidth));
		glDrawElements(GL_TRIANGLES, tilemap->tiles.size() * 6, GL_UNSIGNED_INT, (GLvoid *) 0);
	}

	void ElementBufferedRenderer::reupload() {
		glDeleteVertexArrays(1, &vaoHandle);
		glDeleteBuffers(1, &vboHandle);
		generateVertexBufferObject();
		generateVertexArrayObject();
	}

	bool ElementBufferedRenderer::onBackbufferResized(int width, int height) {
		if (TilemapRenderer::onBackbufferResized(width, height)) {
			generateLightingTexture();
			return true;
		}
		return false;
	}

	void ElementBufferedRenderer::createShader() {
		const GLchar *vert_ptr = reinterpret_cast<const GLchar *>(buffered_vert);
		int vert_handle = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vert_handle, 1, &vert_ptr, reinterpret_cast<const GLint *>(&buffered_vert_len));
		glCompileShader(vert_handle);
		check(vert_handle);

		const GLchar *frag_ptr = reinterpret_cast<const GLchar *>(buffered_frag);
		int frag_handle = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(frag_handle, 1, &frag_ptr, reinterpret_cast<const GLint *>(&buffered_frag_len));
		glCompileShader(frag_handle);
		check(frag_handle);

		shaderHandle = glCreateProgram();
		glAttachShader(shaderHandle, vert_handle);
		glAttachShader(shaderHandle, frag_handle);
		glLinkProgram(shaderHandle);
		check(shaderHandle, true);

		glDetachShader(shaderHandle, vert_handle);
		glDeleteShader(vert_handle);

		glDetachShader(shaderHandle, frag_handle);
		glDeleteShader(frag_handle);
	}

	void ElementBufferedRenderer::generateVertexBufferObject() {
		glGenBuffers(1, &vboHandle);
		glBindBuffer(GL_ARRAY_BUFFER, vboHandle);

		const size_t float_count = tilemap->tiles.size() << 4;
		auto vertex_data = std::make_unique<float[]>(float_count);
		size_t i = 0;

		const auto set_width = tilemap->setWidth / tilemap->tileSize;
		const float divisor = set_width;
		const float ty_size = 1.f / divisor - tileTexturePadding * 2;

		for (int x = 0; x < tilemap->width; ++x)
			for (int y = 0; y < tilemap->height; ++y) {
				const int tile = (*tilemap)(x, y);
				const float tx0 = (tile % set_width) / divisor + tileTexturePadding;
				const float ty0 = (tile / set_width) / divisor + tileTexturePadding;

				// Vertex 0 (top left)
				vertex_data[i + 0] = x; // position x
				vertex_data[i + 1] = y; // position y
				vertex_data[i + 2] = tx0; // texcoord x
				vertex_data[i + 3] = ty0; // texcoord y
				i += 4;

				// Vertex 1 (top right)
				vertex_data[i + 0] = x + 1; // position x
				vertex_data[i + 1] = y;     // position y
				vertex_data[i + 2] = tx0 + ty_size; // texcoord x
				vertex_data[i + 3] = ty0;           // texcoord y
				i += 4;

				// Vertex 2 (bottom left)
				vertex_data[i + 0] = x;     // position x
				vertex_data[i + 1] = y + 1; // position y
				vertex_data[i + 2] = tx0;           // texcoord x
				vertex_data[i + 3] = ty0 + ty_size; // texcoord y
				i += 4;

				// Vertex 3 (bottom right)
				vertex_data[i + 0] = x + 1; // position x
				vertex_data[i + 1] = y + 1; // position y
				vertex_data[i + 2] = tx0 + ty_size; // texcoord x
				vertex_data[i + 3] = ty0 + ty_size; // texcoord y
				i += 4;
			}

		glBufferData(GL_ARRAY_BUFFER, float_count * sizeof(float), vertex_data.get(), GL_STATIC_DRAW);
	}

	void ElementBufferedRenderer::generateElementBufferObject() {
		glGenBuffers(1, &eboHandle);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboHandle);

		size_t index_count = tilemap->tiles.size() * 6;
		auto indices = std::make_unique<unsigned[]>(index_count);

		unsigned i = 0, j = 0;
		for (int x = 0; x < tilemap->width; ++x) {
			for (int y = 0; y < tilemap->height; ++y) {
				indices[i + 0] = j;
				indices[i + 1] = j + 1;
				indices[i + 2] = j + 2;
				indices[i + 3] = j + 1;
				indices[i + 4] = j + 2;
				indices[i + 5] = j + 3;
				i += 6;
				j += 4;
			}
		}

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(unsigned), indices.get(), GL_STATIC_DRAW);
	}

	void ElementBufferedRenderer::generateVertexArrayObject() {
		glGenVertexArrays(1, &vaoHandle);

		glBindVertexArray(vaoHandle);
		glBindBuffer(GL_ARRAY_BUFFER, vboHandle);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (GLvoid *) 0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (GLvoid *) (sizeof(float) * 2));
	}

	void ElementBufferedRenderer::generateLightingFrameBuffer() {
		glGenFramebuffers(1, &lfbHandle);
		generateLightingTexture();
	}

	void ElementBufferedRenderer::generateLightingTexture() {
		if (lfbTexture != 0)
			glDeleteTextures(1, &lfbTexture);

		glGenTextures(1, &lfbTexture);
		glBindTexture(GL_TEXTURE_2D, lfbTexture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, backbufferWidth, backbufferHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	void ElementBufferedRenderer::generateSampler() {
		if (sampler != 0)
			glDeleteSamplers(1, &sampler);
		glGenSamplers(1, &sampler);
	}
}
