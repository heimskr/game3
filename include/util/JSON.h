#pragma once

#include <boost/json/fwd.hpp>

namespace Game3 {
	std::string stringifyWithBigInt(const boost::json::value &);

	boost::json::value * resolveJSON(boost::json::value &, const boost::json::array &path);
	bool patchJSON(boost::json::value &json, const boost::json::array &patch);
}
