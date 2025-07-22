#pragma once

#include "types/UString.h"

#include <optional>

namespace Game3 {
	class HasTooltipText {
		public:
			virtual const std::optional<UString> & getTooltipText() const;
			virtual void setTooltipText(UString);

		protected:
			mutable std::optional<UString> tooltipText;
			bool tooltipTextChanged;
	};
}
