#include "Log.h"
#include "graphics/GL.h"

namespace GL {
	void checkGL(GLenum err, const char *file, int line) {
		if (err) {
			std::cerr << "\e[31mError at " << file << ':' << line << ": " << gluErrorString(err) << "\e[39m\n";
		}
	}

	void VBO::update(const void *data, GLsizeiptr size, bool sub, GLenum usage) {
		if (!bind())
			return;

		if (sub) {
			glBufferSubData(GL_ARRAY_BUFFER, 0, size, data); CHECKGL
		} else {
			glBufferData(GL_ARRAY_BUFFER, size, data, usage); CHECKGL
		}
	}

	FBOBinder FBO::getBinder() {
		return FBOBinder(*this);
	}

	TextureFBOBinder Texture::getBinder() {
		return TextureFBOBinder(*this);
	}

	void TextureFBOBinder::save() {
		static_assert(sizeof(GLuint) == sizeof(GLint));
		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, reinterpret_cast<GLint *>(&handle)); CHECKGL
	}

	void TextureFBOBinder::restore() {
		if (handle == 0)
			unbindFBTexture();
		else
			useTextureInFB(handle);
	}
}
