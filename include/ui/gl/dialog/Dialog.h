#pragma once

#include "graphics/Color.h"
#include "math/Rectangle.h"
#include "types/Types.h"
#include "ui/Modifiers.h"
#include "ui/gl/widget/Widget.h"
#include "util/Concepts.h"

#include <sigc++/sigc++.h>

#include <array>
#include <memory>
#include <string_view>

namespace Game3 {
	class UIContext;
	struct Color;
	struct RendererContext;

	class Dialog;
	using DialogPtr = std::shared_ptr<Dialog>;
	using ConstDialogPtr = std::shared_ptr<const Dialog>;
	using WeakDialogPtr = std::weak_ptr<Dialog>;

	class Dialog: public Widget {
		public:
			sigc::signal<void()> signalClose;

			Dialog(UIContext &, float selfScale);

			virtual ~Dialog() = default;

			SizeRequestMode getRequestMode() const override;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) override;

			virtual void render(const RendererContext &) = 0;
			virtual Rectangle getPosition() const = 0;

			virtual void onClose();
			virtual bool hidesHotbar() const;

			bool click(int button, int x, int y, Modifiers) override;
			bool mouseDown(int button, int x, int y, Modifiers) override;
			bool mouseUp(int button, int x, int y, Modifiers) override;
			bool dragStart(int x, int y) override;
			bool dragUpdate(int x, int y) override;
			bool dragEnd(int x, int y, double) override;
			bool scroll(float x_delta, float y_delta, int x, int y, Modifiers) override;
			bool contains(int x, int y) const override;

			bool isFocused() const;
			void recenter();

		protected:
			DialogPtr getSelf();
			ConstDialogPtr getSelf() const;

		private:
			void render(const RendererContext &, float x, float y, float width, float height) final;
			void render(const RendererContext &, const Rectangle &) final;
	};
}
