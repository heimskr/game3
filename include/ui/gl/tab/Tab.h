#pragma once

#include "ui/gl/widget/Widget.h"

#include <memory>

namespace Game3 {
	class OmniDialog;
	class Texture;
	class UIContext;
	struct RendererContext;

	class Tab: public Widget {
		public:
			Tab(UIContext &);

			virtual void init();
			virtual void renderIcon(const RendererContext &);
			float calculateHeight(const RendererContext &, float available_width, float available_height) override;

		protected:
			UIContext &ui;

			void renderIconTexture(const RendererContext &, const std::shared_ptr<Texture> &);
	};

	using TabPtr = std::shared_ptr<Tab>;
}
