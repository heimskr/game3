#include "ui/gl/HasTooltipText.h"

namespace Game3 {
	const std::optional<UString> & HasTooltipText::getTooltipText() const {
		return tooltipText;
	}

	void HasTooltipText::setTooltipText(UString new_tooltip_text) {
		tooltipText = std::move(new_tooltip_text);
	}
}
