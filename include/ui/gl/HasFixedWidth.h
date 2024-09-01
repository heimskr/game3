#pragma once

namespace Game3 {
	class HasFixedWidth {
		public:
			virtual ~HasFixedWidth() = default;

			virtual float getFixedWidth() const;
			virtual void setFixedWidth(float);

		protected:
			float fixedWidth = -1;

			HasFixedWidth(float);
			HasFixedWidth();
	};
}
