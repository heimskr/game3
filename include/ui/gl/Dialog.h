#pragma once

#include "graphics/Color.h"
#include "graphics/Rectangle.h"
#include "types/Types.h"
#include "ui/Modifiers.h"

#include <array>
#include <string_view>

namespace Game3 {
	class UIContext;
	struct Color;
	struct RendererContext;

	class Dialog {
		protected:
			UIContext &ui;
			Dialog(UIContext &ui);

		public:
			virtual ~Dialog() = default;

			virtual void render(const RendererContext &) = 0;
			virtual Rectangle getPosition() const = 0;
			virtual void onClose();
			virtual bool click(int button, int x, int y);
			virtual bool dragStart(int x, int y);
			virtual bool dragUpdate(int x, int y);
			virtual bool dragEnd(int x, int y);
			virtual bool scroll(float x_delta, float y_delta, int x, int y);
			virtual bool keyPressed(uint32_t character, Modifiers);
	};

	using DialogPtr = std::shared_ptr<Dialog>;
}
