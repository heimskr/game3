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
			Tab(UIContext &, float selfScale);

			virtual void renderIcon(const RendererContext &);

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

		protected:
			void renderIconTexture(const RendererContext &, const std::shared_ptr<Texture> &);
			bool isActive() const;
	};

	using TabPtr = std::shared_ptr<Tab>;
}
