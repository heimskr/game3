#pragma once

#include "graphics/Color.h"
#include "types/UString.h"
#include "ui/widget/Widget.h"
#include "ui/HasFixedSize.h"

namespace Game3 {
	class Autocompleter;
	class Box;
	class Scroller;

	class AutocompleteDropdown: public Widget, public HasFixedSize {
		public:
			AutocompleteDropdown(UIContext &, float scale, Color exterior_color, Color interior_color);
			AutocompleteDropdown(UIContext &, float scale);

			void init() final;
			void render(const RendererContext &, float x, float y, float width, float height) final;

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

			bool checkParent(const std::shared_ptr<Autocompleter> &) const;
			bool checkParent(const Autocompleter &) const;
			void setParent(const std::shared_ptr<Autocompleter> &);

			const std::pair<float, float> & getOrigin() const;
			void setOrigin(std::pair<float, float>);

			void setSuggestions(std::vector<UString>);

			void constrainSize();

		private:
			Color exteriorColor;
			Color interiorColor;
			std::weak_ptr<Autocompleter> weakParent;
			std::shared_ptr<Scroller> scroller;
			std::shared_ptr<Box> vbox;
			std::pair<float, float> origin;
			std::vector<UString> suggestions;

			void choose(const UString &);
	};
}
