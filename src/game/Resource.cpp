#include "game/Resource.h"
#include "threading/ThreadContext.h"

#include <boost/json.hpp>

namespace Game3 {
	Resource::Resource(Identifier identifier_, const boost::json::value &json):
		NamedRegisterable(std::move(identifier_)),
		richnessRange(boost::json::value_to<decltype(richnessRange)>(json.at("richnessRange"))),
		likelihood(boost::json::value_to<decltype(likelihood)>(json.at("likelihood"))),
		cap(findCap(json)) {}

	double Resource::sampleRichness(double factor) const {
		std::uniform_int_distribution distribution{int(factor * richnessRange.first), int(factor * richnessRange.second)};
		return distribution(threadContext.rng) / factor;
	}

	bool Resource::sampleLikelihood() const {
		std::uniform_real_distribution percent(0., 100.);
		return percent(threadContext.rng) < likelihood;
	}

	double Resource::findCap(const boost::json::value &json) {
		if (auto *value = json.as_object().if_contains("cap")) {
			return value->as_double();
		}
		return 100;
	}
}
