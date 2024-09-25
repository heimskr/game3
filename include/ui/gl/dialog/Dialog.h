#pragma once

#include "graphics/Color.h"
#include "graphics/Rectangle.h"
#include "types/Types.h"
#include "ui/Modifiers.h"
#include "ui/gl/widget/Widget.h"

#include <array>
#include <memory>
#include <string_view>

namespace Game3 {
	class UIContext;
	struct Color;
	struct RendererContext;

	class Dialog: public Widget {
		protected:
			Dialog(UIContext &);

		public:
			virtual ~Dialog() = default;

			SizeRequestMode getRequestMode() const override;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) override;

			virtual void render(const RendererContext &) = 0;
			virtual Rectangle getPosition() const = 0;

			virtual void onClose();
			virtual bool click(int button, int x, int y) override;
			virtual bool mouseDown(int button, int x, int y) override;
			virtual bool mouseUp(int button, int x, int y) override;
			virtual bool dragStart(int x, int y) override;
			virtual bool dragUpdate(int x, int y) override;
			virtual bool dragEnd(int x, int y) override;
			virtual bool scroll(float x_delta, float y_delta, int x, int y) override;
			virtual bool hidesHotbar() const;

		protected:
			std::shared_ptr<Dialog> getSelf();
			std::shared_ptr<const Dialog> getSelf() const;

		private:
			void render(const RendererContext &, float x, float y, float width, float height) final;
			void render(const RendererContext &, const Rectangle &) final;
	};

	using DialogPtr = std::shared_ptr<Dialog>;
}
