#pragma once

#include "data/Identifier.h"
#include "registry/Registerable.h"

#include <boost/json/fwd.hpp>

namespace Game3 {
	class Resource: public NamedRegisterable {
		public:
			Resource(Identifier, const boost::json::value &);

			double sampleRichness(double factor = 10) const;
			bool sampleLikelihood() const;
			inline auto getCap() const { return cap; }

		private:
			std::pair<double, double> richnessRange;
			double likelihood{};
			double cap{};

			static double findCap(const boost::json::value &);
	};
}
