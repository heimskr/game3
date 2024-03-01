#pragma once

#include "types/Types.h"

#include <memory>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace Game3 {
	class CombinerInput {
		public:
			std::vector<std::pair<ItemCount, std::string>> inputs;

			CombinerInput() = default;

			std::vector<ItemStackPtr> getStacks(const GamePtr &);

			static CombinerInput fromJSON(const nlohmann::json &, ItemCount *output_count_out = nullptr);
	};
}
