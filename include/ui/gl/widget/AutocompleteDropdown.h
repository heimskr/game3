#pragma once

#include "graphics/Color.h"
#include "types/UString.h"
#include "ui/gl/widget/Widget.h"
#include "ui/gl/HasFixedSize.h"

namespace Game3 {
	class Autocompleter;
	class Box;
	class Scroller;

	class AutocompleteDropdown: public Widget, public HasFixedSize {
		public:
			AutocompleteDropdown(float scale, Color exterior_color, Color interior_color);
			AutocompleteDropdown(float scale);

			void init(UIContext &) final;
			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

			bool checkParent(const std::shared_ptr<Autocompleter> &) const;
			bool checkParent(const Autocompleter &) const;
			void setParent(const std::shared_ptr<Autocompleter> &);

			const std::pair<float, float> & getOrigin() const;
			void setOrigin(std::pair<float, float>);

			void setSuggestions(std::vector<UString>);

			void queueConstrainSize();
			void constrainSize(UIContext &);

		private:
			Color exteriorColor;
			Color interiorColor;
			std::weak_ptr<Autocompleter> weakParent;
			std::shared_ptr<Scroller> scroller;
			std::shared_ptr<Box> vbox;
			std::pair<float, float> origin;
			std::vector<UString> suggestions;
			bool sizeConstrainQueued = false;

			void choose(const UString &);
	};
}
