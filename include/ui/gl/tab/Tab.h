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

			virtual void render(const RendererContext &) = 0;
			virtual void renderIcon(const RendererContext &);
			virtual void click(int button, int x, int y);
			virtual void dragStart(int x, int y);
			virtual void dragUpdate(int x, int y);
			virtual void dragEnd(int x, int y);
			virtual void scroll(float x_delta, float y_delta, int x, int y);

		protected:
			UIContext &ui;

			void renderIconTexture(const RendererContext &, const std::shared_ptr<Texture> &);
	};
}
