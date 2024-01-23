#include "game/Resource.h"
#include "threading/ThreadContext.h"

#include <nlohmann/json.hpp>

namespace Game3 {
	Resource::Resource(Identifier identifier_, const nlohmann::json &json):
		NamedRegisterable(std::move(identifier_)),
		richnessRange(json.at("richnessRange")),
		likelihood(json.at("likelihood")),
		cap(findCap(json)) {}

	double Resource::sampleRichness(double factor) const {
		std::uniform_int_distribution distribution{int(factor * richnessRange.first), int(factor * richnessRange.second)};
		return distribution(threadContext.rng) / factor;
	}

	bool Resource::sampleLikelihood() const {
		std::uniform_real_distribution percent(0., 100.);
		return percent(threadContext.rng) < likelihood;
	}

	double Resource::findCap(const nlohmann::json &json) {
		if (auto iter = json.find("cap"); iter != json.end())
			return iter->get<double>();
		return 100;
	}
}
