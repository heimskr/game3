#pragma once

namespace Game3 {
	class HasFixedHeight {
		public:
			virtual ~HasFixedHeight() = default;

			virtual float getFixedHeight() const;
			virtual void setFixedHeight(float);

		protected:
			float fixedHeight = -1;

			HasFixedHeight(float);
			HasFixedHeight() = default;
	};
}
