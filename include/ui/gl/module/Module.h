#pragma once

#include "ui/gl/widget/Widget.h"

namespace Game3 {
	class Module: public Widget {
		public:
			Module() = default;

			virtual ~Module() = default;

			virtual Identifier getID() const = 0;
			virtual void reset() = 0;
			virtual void update() = 0;
	};
}
