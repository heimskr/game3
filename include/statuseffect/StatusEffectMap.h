#pragma once

#include "statuseffect/StatusEffect.h"

#include <unordered_map>

namespace Game3 {
	class StatusEffectMap: public std::unordered_map<Identifier, std::unique_ptr<StatusEffect>> {};

	class BasicBuffer;
	class Buffer;

	Buffer & operator+=(Buffer &, const StatusEffectMap &);
	Buffer & operator<<(Buffer &, const StatusEffectMap &);
	BasicBuffer & operator>>(BasicBuffer &, StatusEffectMap &);
}
