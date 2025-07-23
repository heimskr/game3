#pragma once

#include "data/Identifier.h"
#include "graphics/GL.h"
#include "graphics/PathmapTextureCache.h"
#include "math/Rectangle.h"
#include "graphics/Texture.h"
#include "ui/UI.h"

#include <any>
#include <memory>
#include <string>

namespace Game3 {
	class BottomDialog;
	class ChatDialog;
	class ClientInventory;
	class HasFluids;
	class OmniDialog;
	class TileEntity;
	class TopDialog;

	class GameUI final: public UI {
		public:
			std::shared_ptr<BottomDialog> bottomDialog;
			std::shared_ptr<ChatDialog> chatDialog;
			std::shared_ptr<OmniDialog> omniDialog;
			std::shared_ptr<TopDialog> topDialog;
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

			static Identifier ID() { return {"base", "ui/game"}; }
			Identifier getID() const final { return ID(); }

			using UI::UI;

			void init(Window &) final;
			void render(const RendererContext &) final;
			Rectangle getPosition() const final;

			bool keyPressed(uint32_t key, Modifiers modifiers, bool is_repeat) final;

			const std::shared_ptr<OmniDialog> & getOmniDialog();
			const std::shared_ptr<ChatDialog> & getChatDialog();
			const std::shared_ptr<TopDialog> & getTopDialog();
			const std::shared_ptr<BottomDialog> & getBottomDialog();

			void showOmniDialog();
			void hideOmniDialog();
			void toggleOmniDialog();

			void openModule(const Identifier &, const std::any &);
			void removeModule();

			void moduleMessageBuffer(const Identifier &module_id, const std::shared_ptr<Agent> &source, const std::string &name, Buffer &&data);

			void showExternalInventory(const std::shared_ptr<ClientInventory> &);
			void showFluids(const std::shared_ptr<HasFluids> &);
			GlobalID getExternalGID() const;

		private:
			std::weak_ptr<TileEntity> hoveredTileEntity;
	};
}
