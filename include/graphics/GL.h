#pragma once

#include <array>
#include <csignal>
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>

#include "graphics/OpenGL.h"

#include "lib/stb/stb_image.h"
#include "lib/stb/stb_image_write.h"

#define HIDE_CHECKGL

#define FORCE_CHECKGL do { GL::checkGL(glGetError(), __FILE__, __LINE__); } while(0);
#define FORCE_CHECKGL_SET GLenum gl_err = glGetError(); GL::checkGL(gl_err, __FILE__, __LINE__);

#ifdef HIDE_CHECKGL
#define CHECKGL
#define CHECKGL_SET GLenum gl_err = 0;
#else
#define CHECKGL FORCE_CHECKGL
#define CHECKGL_SET FORCE_CHECKGL_SET
#endif

namespace GL {
	// TODO: makeRGBTexture

	void checkGL(GLenum err, const char *file, int line);

	inline void useTextureInFB(GLuint texture) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0); CHECKGL
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0); CHECKGL
	}

	inline void unbindFBTexture() {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0); CHECKGL
	}

	inline void bindFB(GLuint framebuffer) {
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer); CHECKGL
	}

	inline void clear(float r, float g, float b, float a = 1.f, GLbitfield field = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) {
		glClearColor(r, g, b, a); CHECKGL
		glClear(field); CHECKGL
	}

	inline GLint getFB() {
		GLint fb = -1;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fb); CHECKGL
		if (fb == -1)
			throw std::runtime_error("Couldn't get current framebuffer binding");
		return fb;
	}

	inline GLuint makeFloatTexture(GLsizei width, GLsizei height, GLint min_filter = GL_LINEAR, GLint mag_filter = GL_LINEAR) {
		GLuint texture = -1;
		glGenTextures(1, &texture); CHECKGL
		if (texture == static_cast<GLuint>(-1))
			throw std::runtime_error("Couldn't generate float texture");
		glBindTexture(GL_TEXTURE_2D, texture); CHECKGL
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr); CHECKGL
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter); CHECKGL
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter); CHECKGL
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); CHECKGL
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); CHECKGL
		return texture;
	}

	inline GLuint makeRGBATexture(GLsizei width, GLsizei height, GLint min_filter = GL_LINEAR, GLint mag_filter = GL_LINEAR) {
		GLuint texture = -1;
		glGenTextures(1, &texture); CHECKGL
		if (texture == static_cast<GLuint>(-1))
			throw std::runtime_error("Couldn't generate RGBA texture");
		glBindTexture(GL_TEXTURE_2D, texture); CHECKGL
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr); CHECKGL
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter); CHECKGL
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter); CHECKGL
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); CHECKGL
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); CHECKGL
		return texture;
	}

	inline GLuint makeTexture(GLenum texture_type, GLenum data_type, GLsizei width, GLsizei height, GLint min_filter = GL_LINEAR, GLint mag_filter = GL_LINEAR) {
		GLuint texture = -1;
		glGenTextures(1, &texture); CHECKGL
		if (texture == static_cast<GLuint>(-1))
			throw std::runtime_error("Couldn't generate texture");
		glBindTexture(GL_TEXTURE_2D, texture); CHECKGL
		glTexImage2D(GL_TEXTURE_2D, 0, texture_type, width, height, 0, GL_RGBA, data_type, nullptr); CHECKGL
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter); CHECKGL
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter); CHECKGL
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); CHECKGL
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); CHECKGL
		return texture;
	}

	inline void deleteTexture(GLuint texture) {
		if (texture != 0) {
			glDeleteTextures(1, &texture); CHECKGL
		}
	}

	inline GLuint makeFBO() {
		GLuint fb = -1;
		glGenFramebuffers(1, &fb); CHECKGL
		if (fb == static_cast<GLuint>(-1))
			throw std::runtime_error("Couldn't generate FBO");
		return fb;
	}

	GLuint makeFloatVAO(GLuint vbo, std::initializer_list<int> sizes);

	template <typename T>
	inline GLuint makeBufferObject(GLenum target, const T *data, size_t count, GLenum usage)  {
		GLuint bo = -1;
		glGenBuffers(1, &bo); CHECKGL
		if (bo == static_cast<GLuint>(-1))
			throw std::runtime_error("Couldn't generate buffer object");
		glBindBuffer(target, bo); CHECKGL
		glBufferData(target, count * sizeof(T), data, usage); CHECKGL
		return bo;
	}

	template <typename T>
	inline GLuint makeEBO(const T *data, size_t count, GLenum usage) {
		return makeBufferObject(GL_ELEMENT_ARRAY_BUFFER, data, count, usage);
	}

	template <typename T>
	inline GLuint makeVBO(const T *data, size_t count, GLenum usage) {
		return makeBufferObject(GL_ARRAY_BUFFER, data, count, usage);
	}

	template <typename T, size_t N>
	inline GLuint genSquareVBO(size_t width, size_t height, GLenum usage, const std::function<std::array<std::array<T, N>, 4>(size_t, size_t)> &fn) {
		std::vector<T> vertex_data;
		vertex_data.reserve(width * height * (2 + N));

		for (size_t x = 0; x < width; ++x) {
			for (size_t y = 0; y < height; ++y) {
				const auto generated = fn(x, y);

				vertex_data.push_back(x);
				vertex_data.push_back(y);
				for (const T item: generated[0])
					vertex_data.push_back(item);

				vertex_data.push_back(x + 1);
				vertex_data.push_back(y);
				for (const T item: generated[1])
					vertex_data.push_back(item);

				vertex_data.push_back(x);
				vertex_data.push_back(y + 1);
				for (const T item: generated[2])
					vertex_data.push_back(item);

				vertex_data.push_back(x + 1);
				vertex_data.push_back(y + 1);
				for (const T item: generated[3])
					vertex_data.push_back(item);
			}
		}

		return GL::makeVBO(vertex_data.data(), vertex_data.size(), usage);
	}

	template <typename T, size_t N>
	inline GLuint genEBO2D(size_t width, size_t height, GLenum usage, const std::function<std::array<T, N>(size_t, size_t)> &fn) {
		std::vector<T> element_data;
		element_data.reserve(width * height * N);

		for (size_t x = 0; x < width; ++x)
			for (size_t y = 0; y < height; ++y)
				for (const T item: fn(x, y))
					element_data.push_back(item);

		return GL::makeEBO(element_data.data(), element_data.size(), usage);
	}

	inline void bindTexture(uint8_t index, GLuint texture, GLenum target = GL_TEXTURE_2D) {
		glActiveTexture(GL_TEXTURE0 + index); CHECKGL
		glBindTexture(target, texture); CHECKGL
	}

	inline void triangles(GLsizei count) {
		glDrawElements(GL_TRIANGLES, count * 6, GL_UNSIGNED_INT, nullptr); CHECKGL
	}

	class VBO {
		public:
			VBO(): handle(0) {}

			VBO(GLuint handle_): handle(handle_) {}

			template <typename... Args>
			VBO(Args &&...args) {
				init(std::forward<Args>(args)...);
			}

			VBO(const VBO &) = delete;
			VBO(VBO &&other): handle(other.handle) {
				other.handle = 0;
			}

			VBO & operator=(const VBO &) = delete;
			VBO & operator=(VBO &&other) {
				handle = other.handle;
				other.handle = 0;
				return *this;
			}

			template <typename T>
			void init(const T *data, size_t count, GLenum usage) {
				reset();
				handle = makeBufferObject(GL_ARRAY_BUFFER, data, count, usage);
			}

			template <typename T>
			void update(const std::vector<T> &data, bool sub = true, GLenum usage = GL_DYNAMIC_DRAW) {
				update(data.data(), sizeof(T) * data.size(), sub, usage);
			}

			template <typename T, size_t N>
			void init(size_t width, size_t height, GLenum usage, const std::function<std::array<std::array<T, N>, 4>(size_t, size_t)> &fn) {
				reset();
				handle = genSquareVBO<T, N>(width, height, usage, fn);
			}

			~VBO() {
				reset();
			}

			inline bool reset() {
				if (handle == 0)
					return false;
				glDeleteBuffers(1, &handle); CHECKGL
				handle = 0;
				return true;
			}

			inline bool bind() {
				if (handle == 0)
					return false;
				glBindBuffer(GL_ARRAY_BUFFER, handle); CHECKGL
				return true;
			}

			inline auto getHandle() const { return handle; }

		protected:
			GLuint handle = 0;

			void update(const void *, GLsizeiptr, bool sub = true, GLenum usage = GL_DYNAMIC_DRAW);
	};

	class VAO {
		public:
			VAO(const VAO &) = delete;
			VAO(VAO &&other): handle(other.handle) {
				other.handle = 0;
			}

			VAO & operator=(const VAO &) = delete;
			VAO & operator=(VAO &&other) {
				handle = other.handle;
				other.handle = 0;
				return *this;
			}

			~VAO() {
				reset();
			}

			inline bool reset() {
				if (handle == 0)
					return false;
				glDeleteVertexArrays(1, &handle); CHECKGL
				handle = 0;
				return true;
			}

			inline bool bind() {
				if (handle == 0)
					return false;
				glBindVertexArray(handle); CHECKGL
				return true;
			}

			inline auto getHandle() const { return handle; }

		protected:
			GLuint handle = 0;
			VAO(GLuint handle_ = 0): handle(handle_) {}
	};

	class FloatVAO: public VAO {
		public:
			FloatVAO(): VAO() {}
			FloatVAO(GLuint vbo, std::initializer_list<int> sizes): VAO(makeFloatVAO(vbo, sizes)) {}
			FloatVAO(const VBO &vbo, std::initializer_list<int> sizes): FloatVAO(vbo.getHandle(), sizes) {}

			inline void init(GLuint vbo, std::initializer_list<int> sizes) {
				reset();
				handle = makeFloatVAO(vbo, sizes);
			}

			inline void init(const VBO &vbo, std::initializer_list<int> sizes) {
				init(vbo.getHandle(), sizes);
			}
	};

	class EBO {
		public:
			EBO(GLuint handle_ = 0): handle(handle_) {}

			template <typename... Args>
			EBO(Args &&...args) {
				init(std::forward<Args>(args)...);
			}

			EBO(const EBO &) = delete;
			EBO(EBO &&) = delete;

			EBO & operator=(const EBO &) = delete;
			EBO & operator=(EBO &&) = delete;

			~EBO() {
				reset();
			}

			inline bool reset() {
				if (handle == 0)
					return false;
				glDeleteBuffers(1, &handle); CHECKGL
				handle = 0;
				return true;
			}

			inline bool bind() {
				if (handle == 0)
					return false;
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle); CHECKGL
				return true;
			}

			template <typename T, size_t N>
			inline void init(size_t width, size_t height, GLenum usage, const std::function<std::array<T, N>(size_t, size_t)> &fn) {
				reset();
				handle = genEBO2D(width, height, usage, fn);
			}

			inline auto getHandle() const { return handle; }

		private:
			GLuint handle = 0;
	};

	class FBOBinder;

	class FBO {
		public:
			FBO() = default;

			FBO(GLuint handle_):
				handle(handle_) {}

			~FBO() {
				reset();
			}

			inline bool reset() {
				if (handle == 0)
					return false;
				glDeleteFramebuffers(1, &handle); CHECKGL
				handle = 0;
				return true;
			}

			inline bool bind() {
				if (handle == 0)
					return false;
				oldBuffer = getFB();
				bindFB(handle);
				return true;
			}

			FBOBinder getBinder();

			inline bool undo() {
				if (handle == 0 || oldBuffer < 0)
					return false;
				unbindFBTexture();
				bindFB(oldBuffer);
				oldBuffer = -1;
				return true;
			}

			inline bool init(bool force = false) {
				if (handle != 0) {
					if (!force)
						return false;
					reset();
				}
				handle = makeFBO();
				return true;
			}

			inline auto getHandle() const {
				return handle;
			}

			inline bool isBound() const {
				return 0 <= oldBuffer;
			}

		private:
			GLint oldBuffer = -1;
			GLuint handle = 0;
	};

	class FBOBinder {
		public:
			explicit FBOBinder(FBO &fbo_): fbo(fbo_) {
				if (fbo.isBound())
					active = false;
				else
					fbo.bind();
			}

			~FBOBinder() {
				if (active)
					fbo.undo();
			}

			void undo() {
				if (active) {
					active = false;
					fbo.undo();
				}
			}

		private:
			FBO &fbo;
			bool active = true;
	};

	class TextureFBOBinder;

	class Texture {
		public:
			Texture() = default;

			Texture(GLuint handle_, GLsizei width_, GLsizei height_):
				handle(handle_), width(width_), height(height_) {}

			Texture(const Texture &) = delete;
			Texture(Texture &&other) {
				handle = other.handle;
				width  = other.width;
				height = other.height;
				other.handle = 0;
				other.width  = 0;
				other.height = 0;
			}

			Texture & operator=(const Texture &) = delete;
			Texture & operator=(Texture &&other) {
				reset();
				handle = other.handle;
				width  = other.width;
				height = other.height;
				other.handle = 0;
				other.width  = 0;
				other.height = 0;
				return *this;
			}

			~Texture() {
				reset();
			}

			inline bool reset() {
				if (handle == 0)
					return false;
				glDeleteTextures(1, &handle); CHECKGL
				handle = 0;
				return true;
			}

			inline bool bind(uint8_t index, GLenum target = GL_TEXTURE_2D) {
				if (handle == 0)
					return false;
				GL::bindTexture(index, handle, target);
				return true;
			}

			TextureFBOBinder getBinder();

			inline bool initFloat(GLsizei width_, GLsizei height_, GLint min_filter = GL_LINEAR, GLint mag_filter = GL_LINEAR) {
				reset();
				handle = GL::makeFloatTexture(width_, height_, min_filter, mag_filter);
				width  = width_;
				height = height_;
				return true;
			}

			inline bool initRGBA(GLsizei width_, GLsizei height_, GLint min_filter = GL_LINEAR, GLint mag_filter = GL_LINEAR) {
				reset();
				handle = GL::makeRGBATexture(width_, height_, min_filter, mag_filter);
				width  = width_;
				height = height_;
				return true;
			}

			inline bool init(GLenum texture_type, GLenum data_type, GLsizei width_, GLsizei height_, GLint min_filter = GL_LINEAR, GLint mag_filter = GL_LINEAR) {
				reset();
				handle = GL::makeTexture(texture_type, data_type, width_, height_, min_filter, mag_filter);
				width  = width_;
				height = height_;
				return true;
			}

			inline bool useInFB() {
				if (handle == 0)
					return false;
				GL::useTextureInFB(handle);
				return true;
			}

			inline explicit operator bool() const {
				return handle != 0;
			}

			inline auto getHandle() const { return handle; }
			inline auto getWidth()  const { return width;  }
			inline auto getHeight() const { return height; }

			template <typename... Args>
			static Texture makeFloat(Args &&...args) {
				Texture out;
				out.initFloat(std::forward<Args>(args)...);
				return out;
			}

			inline bool dump(FBO &fbo, const std::filesystem::path &path) {
				auto pixels = std::make_unique<uint8_t[]>(width * height * 4);
				fbo.bind();
				useInFB();
				glReadBuffer(GL_COLOR_ATTACHMENT0);
				glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.get());
				const bool out = stbi_write_png(path.string().c_str(), width, height, 4, pixels.get(), width * 4);
				fbo.undo();
				return out;
			}

		private:
			GLuint handle = 0;
			GLsizei width = 0;
			GLsizei height = 0;
	};

	class TextureFBOBinder {
		public:
			TextureFBOBinder(Texture &texture) {
				save();
				texture.useInFB();
			}

			~TextureFBOBinder() {
				restore();
			}

		private:
			GLuint handle = 0;

			void save();
			void restore();
	};

	struct Viewport {
		GLint saved[4];

		Viewport() {
			glGetIntegerv(GL_VIEWPORT, saved); CHECKGL
		}

		Viewport(GLint x, GLint y, GLsizei width, GLsizei height) {
			glGetIntegerv(GL_VIEWPORT, saved); CHECKGL
			reframe(x, y, width, height);
		}

		inline void reframe(GLint x, GLint y, GLsizei width, GLsizei height) {
			glViewport(x, y, width, height); CHECKGL
		}

		inline void reset() {
			glViewport(saved[0], saved[1], static_cast<GLsizei>(saved[2]), static_cast<GLsizei>(saved[3])); CHECKGL
		}
	};

	struct Scissor {
		GLint saved[4];

		Scissor();
		Scissor(GLint x, GLint y, GLsizei width, GLsizei height);

		void reframe(GLint x, GLint y, GLsizei width, GLsizei height);
		void reset();
	};
}
