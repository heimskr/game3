#pragma once

#include "data/Identifier.h"
#include "registry/Registerable.h"

#include <nlohmann/json_fwd.hpp>

namespace Game3 {
	class Resource: public NamedRegisterable {
		public:
			Resource(Identifier, const nlohmann::json &);

			double sampleRichness(double factor = 10) const;

		private:
			std::pair<double, double> richnessRange;
	};
}
