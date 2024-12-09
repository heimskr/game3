#pragma once

#include <boost/json/fwd.hpp>

namespace Game3 {
	struct VillageOptions {
		int width   = 34;
		int height  = 26;
		int padding = 2;

		VillageOptions() = default;
		VillageOptions(int width_, int height_, int padding_);
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &, const VillageOptions &);
	VillageOptions tag_invoke(boost::json::value_to_tag<VillageOptions>, const boost::json::value &);
}
