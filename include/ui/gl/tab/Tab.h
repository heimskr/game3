#pragma once

#include <memory>

namespace Game3 {
	class OmniDialog;
	class Texture;
	class UIContext;
	struct RendererContext;

	class Tab {
		public:
			Tab(UIContext &);

			virtual ~Tab() = default;

			virtual void render(UIContext &, RendererContext &) = 0;
			virtual void renderIcon(RendererContext &);
			virtual void click(int x, int y);
			virtual void dragStart(int x, int y);
			virtual void dragEnd(int x, int y);

		protected:
			UIContext &ui;

			void renderIconTexture(RendererContext &, const std::shared_ptr<Texture> &);
	};
}
