#pragma once

/*
	NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
	The widget drawing code is based on the NanoVG demo application
	by Mikko Mononen.

	All rights reserved. Use of this source code is governed by a
	BSD-style license that can be found at https://github.com/wjakob/nanogui/blob/master/LICENSE.txt.
*/

#include <memory>

#include <nanogui/opengl.h>
#include <stb/stb_image.h>

class GLTexture {
	private:
		std::string textureName_;
		GLuint textureID;

	public:
		using handleType = std::unique_ptr<uint8_t[], void(*)(void *)>;

		GLTexture() = default;

		GLTexture(const std::string &texture_name):
			textureName_(texture_name), textureID(0) {}

		GLTexture(const std::string &texture_name, GLint texture_id):
			textureName_(texture_name), textureID(texture_id) {}

		GLTexture(const GLTexture &other) = delete;

		GLTexture(GLTexture &&other) noexcept: textureName_(std::move(other.textureName_)), textureID(other.textureID) {
			other.textureID = 0;
		}

		GLTexture& operator=(const GLTexture& other) = delete;

		GLTexture& operator=(GLTexture&& other) noexcept {
			textureName_ = std::move(other.textureName_);
			std::swap(textureID, other.textureID);
			return *this;
		}

		~GLTexture() noexcept {
			if (textureID)
				glDeleteTextures(1, &textureID);
		}

		GLuint texture() const {
			return textureID;
		}

		const std::string & textureName() const {
			return textureName_;
		}

		/** Load a file in memory and create an OpenGL texture.
		 *  Returns a handle type (an std::unique_ptr) to the loaded pixels. */
		handleType load(const std::string &filename) {
			if (textureID) {
				glDeleteTextures(1, &textureID);
				textureID = 0;
			}
			int force_channels = 0;
			int w, h, n;
			handleType texture_data(stbi_load(filename.c_str(), &w, &h, &n, force_channels), stbi_image_free);
			if (!texture_data)
				throw std::invalid_argument("Could not load texture data from file " + filename);
			glGenTextures(1, &textureID);
			glBindTexture(GL_TEXTURE_2D, textureID);
			GLint internalFormat;
			GLint format;
			switch (n) {
				case 1: internalFormat = GL_R8;    format = GL_RED;  break;
				case 2: internalFormat = GL_RG8;   format = GL_RG;   break;
				case 3: internalFormat = GL_RGB8;  format = GL_RGB;  break;
				case 4: internalFormat = GL_RGBA8; format = GL_RGBA; break;
				default: internalFormat = 0; format = 0; break;
			}
			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, texture_data.get());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			return texture_data;
		}
};