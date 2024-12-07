#pragma once

#include "types/Types.h"

#include <memory>
#include <vector>

#include <boost/json/fwd.hpp>

namespace Game3 {
	class CombinerInput {
		public:
			std::vector<std::pair<ItemCount, std::string>> inputs;

			CombinerInput() = default;

			std::vector<ItemStackPtr> getStacks(const GamePtr &);
	};

	CombinerInput tag_invoke(boost::json::value_to_tag<CombinerInput>, const boost::json::value &, ItemCount *output_count_out = nullptr);
}
