#pragma once

#include <cstdint>

namespace Game3 {
	using Radius = uint64_t;

	class HasRadius {
		public:
			HasRadius() = default;
			HasRadius(Radius radius): radius(radius) {}
			virtual Radius getRadius() const { return radius; }
			virtual void setRadius(Radius new_radius) { radius = new_radius; }

		protected:
			Radius radius = 1;
	};
}
