#include "game/Resource.h"
#include "threading/ThreadContext.h"

#include <nlohmann/json.hpp>

namespace Game3 {
	Resource::Resource(Identifier identifier_, const nlohmann::json &json):
		NamedRegisterable(std::move(identifier_)),
		richnessRange(json.at("richnessRange")) {}

	double Resource::sampleRichness(double factor) const {
		std::uniform_int_distribution distribution{int(factor * richnessRange.first), int(factor * richnessRange.second)};
		return distribution(threadContext.rng) / factor;
	}
}
