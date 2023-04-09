#include <iostream>

#include "Shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "resources.h"
#include "Tiles.h"
#include "realm/Realm.h"
#include "ui/ElementBufferedRenderer.h"
#include "util/GL.h"
#include "util/Timer.h"
#include "util/Util.h"

// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Renderers/ElementBufferedRenderer.cs

namespace Game3 {
	ElementBufferedRenderer::ElementBufferedRenderer(): reshader(blur_frag) {}

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
			glDeleteTextures(1, &lfbBlurredTexture);
			glDeleteFramebuffers(1, &lfbHandle);
			tilemap.reset();
			initialized = false;
		}
	}

	void ElementBufferedRenderer::init(TilemapPtr tilemap_, const TileSet &tileset) {
		if (initialized)
			reset();
		tilemap = std::move(tilemap_);
		createShader();
		generateVertexBufferObject();
		generateElementBufferObject();
		generateVertexArrayObject();
		generateLightingFrameBuffer();
		const auto bright_shorts = tileset.getBright();
		brightTiles.assign(bright_shorts.begin(), bright_shorts.end());
		brightTiles.resize(8, -1);
		brightSet = {bright_shorts.begin(), bright_shorts.end()};
		initialized = true;
	}

	void ElementBufferedRenderer::render(float divisor) {
		if (!initialized)
			return;
		tilemap->texture.bind();

		if (dirty) {
			recomputeLighting();
			dirty = false;
		}

		glActiveTexture(GL_TEXTURE1); CHECKGL
		glBindTexture(GL_TEXTURE_2D, lfbTexture); CHECKGL
		glBindVertexArray(vaoHandle); CHECKGL
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboHandle); CHECKGL

		glm::mat4 projection(1.f);
		projection = glm::scale(projection, {tilemap->tileSize, -tilemap->tileSize, 1}) *
		             glm::scale(projection, {scale / backbufferWidth, scale / backbufferHeight, 1}) *
		             glm::translate(projection, {center.x() - tilemap->width / 2.f, center.y() - tilemap->height / 2.f, 0});

		glUseProgram(shaderHandle); CHECKGL

		glUniform1i(glGetUniformLocation(shaderHandle, "texture0"), 0); CHECKGL
		glUniform1i(glGetUniformLocation(shaderHandle, "texture1"), 1); CHECKGL
		glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "projection"), 1, GL_FALSE, glm::value_ptr(projection)); CHECKGL
		glUniform1f(glGetUniformLocation(shaderHandle,  "divisor"), divisor); CHECKGL
		glUniform1iv(glGetUniformLocation(shaderHandle, "bright_tiles"), brightTiles.size(), brightTiles.data()); CHECKGL
		glUniform1i(glGetUniformLocation(shaderHandle,  "tile_size"), static_cast<GLint>(tilemap->tileSize)); CHECKGL
		glUniform1i(glGetUniformLocation(shaderHandle,  "tileset_size"), static_cast<GLint>(tilemap->setWidth)); CHECKGL
		glUniform2f(glGetUniformLocation(shaderHandle,  "map_size"), static_cast<GLfloat>(tilemap->width), static_cast<GLfloat>(tilemap->height)); CHECKGL
		glDrawElements(GL_TRIANGLES, tilemap->size() * 6, GL_UNSIGNED_INT, (GLvoid *) 0); CHECKGL
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
		const auto set_width = tilemap->setWidth / tilemap->tileSize;
		const float divisor = set_width;
		const float t_size = 1.f / divisor - tileTexturePadding * 2;

		vboHandle = GL::makeSquareVBO<float, 3>(size_t(tilemap->width), size_t(tilemap->height), GL_STATIC_DRAW, [this, set_width, divisor, t_size](size_t x, size_t y) -> std::array<std::array<float, 3>, 4> {
			const auto tile = (*tilemap)(x, y);
			const float tx0 = (tile % set_width) / divisor + tileTexturePadding;
			const float ty0 = (tile / set_width) / divisor + tileTexturePadding;
			const float tile_f = static_cast<float>(tile);
			return std::array {
				std::array {tx0,          ty0,          tile_f},
				std::array {tx0 + t_size, ty0,          tile_f},
				std::array {tx0,          ty0 + t_size, tile_f},
				std::array {tx0 + t_size, ty0 + t_size, tile_f},
			};
		});
	}

	void ElementBufferedRenderer::generateElementBufferObject() {
		size_t index_count = tilemap->size() * 6;
		auto indices = std::make_unique<uint32_t[]>(index_count);

		uint32_t i = 0, j = 0;
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

		eboHandle = GL::makeEBO(indices.get(), index_count, GL_STATIC_DRAW);
	}

	void ElementBufferedRenderer::generateVertexArrayObject() {
		vaoHandle = GL::makeFloatVAO<3>(vboHandle, {2, 2, 1});
	}

	void ElementBufferedRenderer::generateLightingFrameBuffer() {
		lfbHandle = GL::makeFBO();
		generateLightingTexture();
	}

	void ElementBufferedRenderer::generateLightingTexture() {
		GL::deleteTexture(lfbTexture);
		GL::deleteTexture(lfbBlurredTexture);

		const auto width  = tilemap->tileSize * tilemap->width;
		const auto height = tilemap->tileSize * tilemap->height;
		constexpr GLint filter = GL_NEAREST;

		lfbTexture = GL::makeFloatTexture(width, height, filter);
		lfbBlurredTexture = GL::makeFloatTexture(width, height, filter);

		rectangle.update(width, height);
		reshader.update(width, height);
	}

	void ElementBufferedRenderer::recomputeLighting() {
		if (!tilemap)
			return;

		Timer::clear();
		Timer timer("RecomputeLighting");

		bool recomputation_needed = false;
		if (tileCache.empty()) {
			tileCache = tilemap->tiles;
			recomputation_needed = true;
		} else {
			assert(tileCache.size() == tilemap->size());
			for (size_t i = 0, max = tileCache.size(); i < max; ++i) {
				if (brightSet.contains(tileCache[i]) != brightSet.contains(tilemap->tiles[i])) {
					tileCache = tilemap->tiles;
					recomputation_needed = true;
					break;
				}
			}
		}

		if (recomputation_needed) {
			const auto gtk_buffer = GL::getFB();
			GL::bindFB(lfbHandle);

			generateLightingTexture();

			const auto tilesize = tilemap->tileSize;
			const auto width    = tilesize * tilemap->width;
			const auto height   = tilesize * tilemap->height;
			GL::Viewport viewport(0, 0, width, height);

			GL::useTextureInFB(lfbTexture);

			// Clearing to half-white because the color in the lightmap will be multiplied by two
			GL::clear(.5f, .5f, .5f, 0.f);

			Timer lava1("Lava1");
			for (Index row = 0; row < tilemap->height; ++row) {
				for (Index column = 0; column < tilemap->width; ++column) {
					const Position pos(row, column);
					const auto tile = (*tilemap)(pos);
					if (tile == Monomap::LAVA) {
						const float x = column * tilesize;
						const float y = row * tilesize;
						constexpr float radius = 1.5f;
						rectangle.drawOnScreen({1.f, .5f, 0.f, .5f}, x - radius * tilesize, y - radius * tilesize, (2.f * radius + 1.f) * tilesize, (2.f * radius + 1.f) * tilesize);
					}
				}
			}
			lava1.stop();

			reshader.bind();
			reshader.set("xs", static_cast<float>(width));
			reshader.set("ys", static_cast<float>(height));
			reshader.set("r", 5.f);

			Timer blur("Blur");
			for (int i = 0; i < 8; ++i) {
				GL::useTextureInFB(lfbBlurredTexture);
				reshader.set("axis", 0);
				reshader(lfbTexture);

				GL::useTextureInFB(lfbTexture);
				reshader.set("axis", 1);
				reshader(lfbBlurredTexture);
			}
			blur.stop();

			Timer lava2("Lava2");
			for (Index row = 0; row < tilemap->height; ++row) {
				for (Index column = 0; column < tilemap->width; ++column) {
					const Position pos(row, column);
					const auto tile = (*tilemap)(pos);
					if (tile == Monomap::LAVA) {
						const float x = column * tilesize;
						const float y = row * tilesize;
						const float margin = .0f * tilesize;
						rectangle.drawOnScreen({1.f, 1.f, 1.f, 1.f}, x + margin, y + margin, tilesize - 2 * margin, tilesize - 2 * margin);
					}
				}
			}
			lava2.stop();

			viewport.reset();
			GL::bindFB(gtk_buffer);
		}

		timer.stop();
		Timer::summary();
	}
}
