#pragma once

#include "ui/HasFixedHeight.h"
#include "ui/HasFixedWidth.h"

#include <utility>

namespace Game3 {
	class HasFixedSize: public HasFixedWidth, public HasFixedHeight {
		public:
			virtual ~HasFixedSize() = default;

			virtual std::pair<float, float> getFixedSize() const;
			virtual void setFixedSize(float, float);
			virtual void setFixedSize(float);
			virtual void fixSizes(float &width, float &height);
			virtual void fixSizes(float &width, float &height, float scale);

		protected:
			HasFixedSize(float fixed_width, float fixed_height);
			HasFixedSize(float fixed_size);
			HasFixedSize();
	};
}
