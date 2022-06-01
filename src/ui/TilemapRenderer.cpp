#include <iostream>

#include "ui/TilemapRenderer.h"

namespace Game3 {
	void TilemapRenderer::onBackBufferResized(int width, int height) {
		if (width == backBufferWidth && height == backBufferHeight)
			return;
		backBufferWidth = width;
		backBufferHeight = height;
	}
	
	void TilemapRenderer::check(int handle, bool is_link) {
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
}
