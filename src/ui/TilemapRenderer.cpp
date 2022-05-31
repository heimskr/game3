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
		// glClipControl(GL_UPPER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		createShader();
		generateVertexBufferObject();
		generateVertexArrayObject();
	}

	void TilemapRenderer::render() {
		// glClearColor(0.5f, 0.f, 0.f, 1.f);
		// glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(shaderHandle);
		std::cerr << __FILE__ << ':' << __LINE__ << ": !? " << glGetError() << '\n';
		glBindTexture(GL_TEXTURE_2D, tilemap->handle);
		std::cerr << __FILE__ << ':' << __LINE__ << ": !? " << glGetError() << '\n';
		glBindVertexArray(vaoHandle);
		std::cerr << __FILE__ << ':' << __LINE__ << ": !? " << glGetError() << '\n';
		glm::mat4 projection(1.f);
		// glm::mat4 projection;
		// center.x() = (rand() % 1000) - 500;
		// center.y() = (rand() % 1000) - 500;
		// center.x() = (rand() % 1000) / 500.f - 1.0f;
		// center.y() = (rand() % 1000) / 500.f - 1.0f;
		center = {1.f, 1.f};
		// center.x() = float(backBufferWidth / 2 - 640 / 2) / backBufferWidth;
		// center.y() = float(backBufferHeight / 2 - 640 / 2) / backBufferHeight;
		center.x() = 0.5f - 320.f / backBufferWidth;
		center.y() = 0.5f - 320.f / backBufferHeight;
		std::cout << backBufferWidth << " ~ " << backBufferHeight << '\n';
		// center = nanogui::Vector2f(-0.5f, -0.5f);
		// tilemap->tileSize = rand() % 100;
		projection = glm::translate(projection, {-center.x(), -center.y(), 0}) *
					 glm::scale(projection, {tilemap->tileSize, tilemap->tileSize, 1}) *
					//  glm::scale(projection, {0.02f, 0.02f, 1.f});
					 glm::scale(projection, {1.f / backBufferWidth, 1.f / backBufferHeight, 1});
		auto vptr = glm::value_ptr(projection);
		// vptr[0] = 1.f;
		// vptr[1 * 4 + 1] = 1.f;
		// vptr[3 * 4 + 0] = 1.f;
		// vptr[3 * 4 + 1] = 1.f;
		for (int y = 0; y < 4; ++y) {
			for (int x = 0; x < 4; ++x) {
				// projection[x][y] *= 200.f;
				// if (projection[x][y] < 0)
				// 	projection[x][y] = -projection[x][y];
				std::cerr << projection[x][y] << ' ';
			}
			std::cerr << '\n';
		}


		std::cerr << __FILE__ << ':' << __LINE__ << ": !? " << glGetError() << '\n';
		std::cerr << "shaderHandle[" << shaderHandle << "]\n";
		std::cerr << "projection[" << glGetUniformLocation(shaderHandle, "projection") << "]\n";
		std::cerr << "mapSize[" << glGetUniformLocation(shaderHandle, "mapSize") << "]\n";
		std::cerr << "texture0[" << glGetUniformLocation(shaderHandle, "texture0") << "]\n";
		std::cerr << __FILE__ << ':' << __LINE__ << ": !? " << glGetError() << '\n';
		glUseProgram(shaderHandle);
		glUniformMatrix4fv(glGetUniformLocation(shaderHandle, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		std::cerr << __FILE__ << ':' << __LINE__ << ": !? " << glGetError() << '\n';
		std::cerr << "tilemap size(" << tilemap->width << ", " << tilemap->height << ")\n";
		glUniform2i(glGetUniformLocation(shaderHandle, "mapSize"), tilemap->width, tilemap->height);
		GLenum err;
		if ((err = glGetError())) {
			std::cerr << "Error " << err << '\n' << gluErrorString(err) << '\n';
			std::cerr << "mapSize location: " << glGetUniformLocation(shaderHandle, "mapSize") << '\n';
		}
		std::cerr << __FILE__ << ':' << __LINE__ << ": !? " << glGetError() << '\n';
		glDrawArrays(GL_POINTS, 0, tilemap->tiles.size());
		std::cerr << __FILE__ << ':' << __LINE__ << ": !? " << glGetError() << '\n';
	}

	void TilemapRenderer::onBackBufferResized(int width, int height) {
		if (width == backBufferWidth && height == backBufferHeight)
			return;
		backBufferWidth = width;
		backBufferHeight = height;
		// TODO: is this correct? Is this already handled by nanogui?
		glViewport(0, 0, width, height);
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
		std::cerr << ":: shaderHandle[" << shaderHandle << "]\n";
		std::cerr << ":: vert_handle[" << vert_handle << "]\n";
		std::cerr << ":: geom_handle[" << geom_handle << "]\n";
		std::cerr << ":: frag_handle[" << frag_handle << "]\n";
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
