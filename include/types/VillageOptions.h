#pragma once

#include <nlohmann/json_fwd.hpp>

namespace Game3 {
	struct VillageOptions {
		int width   = 34;
		int height  = 26;
		int padding = 2;

		VillageOptions() = default;
		VillageOptions(int width_, int height_, int padding_);
	};

	void to_json(nlohmann::json &, const VillageOptions &);
	void from_json(const nlohmann::json &, VillageOptions &);
}
