#pragma once

#include "graphics/GL.h"
#include "graphics/PathmapTextureCache.h"
#include "graphics/Rectangle.h"
#include "graphics/Texture.h"
#include "ui/UI.h"

namespace Game3 {
	class GameUI final: public UI {
		public:
			PathmapTextureCache pathmapTextureCache;
			GL::Texture mainGLTexture;
			GL::Texture staticLightingTexture;
			GL::Texture dynamicLightingTexture;
			GL::Texture scratchGLTexture;
			GL::Texture causticsGLTexture;
			TexturePtr mainTexture;
			TexturePtr scratchTexture;
			GL::FBO fbo;
			Rectangle realmBounds;

			using UI::UI;

			void init(Window &) final;
			void render(const RendererContext &) final;
			Rectangle getPosition() const final;
	};
}
