#pragma once

#include "ui/gl/HasFixedHeight.h"
#include "ui/gl/HasFixedWidth.h"

#include <utility>

namespace Game3 {
	class HasFixedSize: public HasFixedWidth, public HasFixedHeight {
		public:
			virtual std::pair<float, float> getFixedSize() const;
			virtual void setFixedSize(float, float);
			virtual void setFixedSize(float);
			virtual void fixSizes(float &width, float &height);

		protected:
			HasFixedSize(float fixed_width, float fixed_height);
			HasFixedSize(float fixed_size);
			HasFixedSize();
	};
}
