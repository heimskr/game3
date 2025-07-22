#pragma once

#include "graphics/Color.h"
#include "types/Types.h"
#include "types/UString.h"
#include "ui/widget/Widget.h"
#include "ui/HasFixedSize.h"

#include <sigc++/sigc++.h>

namespace Game3 {
	class Checkbox: public Widget, public HasFixedSize {
		public:
			sigc::signal<void(bool)> onCheck;

			Checkbox(UIContext &, float selfScale, Color top_color, Color bottom_color, Color check_color, Color interior_color);
			Checkbox(UIContext &, float selfScale, Color primary_color, Color interior_color);
			Checkbox(UIContext &, float selfScale);

			using Widget::render;
			void render(const RendererContext &, float x, float y, float width, float height) final;

			bool click(int button, int x, int y, Modifiers) final;

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

			bool getChecked() const;
			void setChecked(bool);

		private:
			bool checked = false;
			Color topColor;
			Color bottomColor;
			Color checkColor;
			Color interiorColor;

			float getTopFraction() const;
	};
}
