#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "ui/gl/widget/AutocompleteDropdown.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/widget/Scroller.h"
#include "ui/gl/Autocompleter.h"
#include "ui/gl/UIContext.h"

namespace Game3 {
	namespace {
		constexpr Color DEFAULT_EXTERIOR_COLOR{"#604620"};
		constexpr Color DEFAULT_INTERIOR_COLOR{"#ddbc82d0"};
	}

	AutocompleteDropdown::AutocompleteDropdown(float scale, Color exterior_color, Color interior_color):
		Widget(scale), exteriorColor(exterior_color), interiorColor(interior_color) {}

	AutocompleteDropdown::AutocompleteDropdown(float scale):
		AutocompleteDropdown(scale, DEFAULT_EXTERIOR_COLOR, DEFAULT_INTERIOR_COLOR) {}

	void AutocompleteDropdown::init(UIContext &) {
		scroller = std::make_shared<Scroller>(scale);
		vbox = std::make_shared<Box>(scale, Orientation::Vertical, 0, 0, Color{});
		scroller->setChild(vbox);
		scroller->insertAtEnd(shared_from_this());
	}

	void AutocompleteDropdown::render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) {
		fixSizes(width, height);
		x = origin.first;
		y = origin.second;

		Widget::render(ui, renderers, x, y, width, height);

		RectangleRenderer &rectangler = renderers.rectangle;

		// Interior
		rectangler.drawOnScreen(interiorColor, x + scale, y, width - 2 * scale, height - scale);

		scroller->render(ui, renderers, x + scale, y, width - 2 * scale, height - scale);

		// Left side
		rectangler.drawOnScreen(exteriorColor, x, y, scale, height - scale);
		// Right side
		rectangler.drawOnScreen(exteriorColor, x + width - scale, y, scale, height - scale);
		// Bottom side
		rectangler.drawOnScreen(exteriorColor, x + scale, y + height - scale, width - 2 * scale, scale);
		// Bottom left corner
		rectangler.drawOnScreen(exteriorColor, x + scale, y + height - 2 * scale, scale, scale);
		// Bottom right corner
		rectangler.drawOnScreen(exteriorColor, x + width - 2 * scale, y + height - 2 * scale, scale, scale);

		if (sizeConstrainQueued) {
			sizeConstrainQueued = false;
			constrainSize(ui);
		}
	}

	SizeRequestMode AutocompleteDropdown::getRequestMode() const {
		return SizeRequestMode::ConstantSize;
	}

	void AutocompleteDropdown::measure(const RendererContext &, Orientation orientation, float, float, float &minimum, float &natural) {
		minimum = natural = orientation == Orientation::Horizontal? fixedWidth : fixedHeight;
	}

	bool AutocompleteDropdown::checkParent(const std::shared_ptr<Autocompleter> &autocompleter) const {
		return weakParent.lock() == autocompleter;
	}

	bool AutocompleteDropdown::checkParent(const Autocompleter &autocompleter) const {
		return weakParent.lock().get() == &autocompleter;
	}

	void AutocompleteDropdown::setParent(const std::shared_ptr<Autocompleter> &autocompleter) {
		weakParent = autocompleter;
	}

	const std::pair<float, float> & AutocompleteDropdown::getOrigin() const {
		return origin;
	}

	void AutocompleteDropdown::setOrigin(std::pair<float, float> new_origin) {
		origin = new_origin;
	}

	void AutocompleteDropdown::setSuggestions(std::vector<UString> new_suggestions) {
		suggestions = std::move(new_suggestions);
		vbox->clearChildren();
		for (const UString &suggestion: suggestions) {
			auto label = std::make_shared<Label>(scale);
			label->setText(suggestion);
			label->setOnClick([this, weak = std::weak_ptr(label)](Widget &, UIContext &, int button, int, int) {
				if (auto label = weak.lock(); label && button == 1)
					choose(label->getText());
				return true;
			});
			label->insertAtEnd(vbox);
		}
		scroller->setChild(vbox);
	}

	void AutocompleteDropdown::queueConstrainSize() {
		sizeConstrainQueued = true;
	}

	void AutocompleteDropdown::constrainSize(UIContext &ui) {
		if (!scroller)
			return;

		WidgetPtr child = scroller->getFirstChild();

		if (!child)
			return;

		float minimum{}, natural{};
		child->measure(ui.getRenderers(), Orientation::Vertical, fixedWidth, fixedHeight, minimum, natural);
		if (0 < natural) {
			setFixedHeight(natural);
		}
	}

	void AutocompleteDropdown::choose(const UString &choice) {
		auto parent = weakParent.lock();
		assert(parent);
		parent->autocomplete(choice);
	}
}
