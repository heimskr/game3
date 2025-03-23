#pragma once

#define GL_DO_NOT_WARNIF_MULTI_GL_VERSION_HEADERS_INCLUDED 1
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#define HIDE_CHECKGL
#include <OpenGL/gl3.h>
#include <OpenGL/glu.h>
#elif defined(__MINGW32__)
#include <GL/glew.h>
#include <GL/wglext.h>
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <GLFW/glfw3.h>
