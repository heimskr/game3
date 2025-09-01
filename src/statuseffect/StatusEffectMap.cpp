#include "game/Game.h"
#include "net/Buffer.h"
#include "statuseffect/StatusEffectFactory.h"
#include "statuseffect/StatusEffectMap.h"

namespace Game3 {
	template <>
	std::string BasicBuffer::getType(const StatusEffectMap &, bool) {
		return {'\xea'};
	}

	Buffer & operator+=(Buffer &buffer, const StatusEffectMap &map) {
		buffer.appendType(map, false);
		buffer += static_cast<uint32_t>(map.size());
		for (const auto &[identifier, effect]: map) {
			buffer << identifier;
			effect->encode(buffer);
		}
		return buffer;
	}

	Buffer & operator<<(Buffer &buffer, const StatusEffectMap &map) {
		return buffer += map;
	}

	BasicBuffer & operator>>(BasicBuffer &buffer, StatusEffectMap &map) {
		GamePtr game = std::dynamic_pointer_cast<Game>(buffer.context.lock());
		assert(game != nullptr);
		auto &registry = game->registry<StatusEffectFactoryRegistry>();

		const std::string type = buffer.popType();
		if (!Buffer::typesMatch(type, buffer.getType(map, false))) {
			buffer.debug();
			throw std::invalid_argument("Invalid type (" + hexString(type, true) + ") in buffer (expected ea)");
		}

		const uint32_t size = popBuffer<uint32_t>(buffer);
		map.clear();
		map.reserve(size);

		for (uint32_t i = 0; i < size; ++i) {
			Identifier identifier = buffer.take<Identifier>();
			std::unique_ptr<StatusEffect> effect = (*registry.at(identifier))();
			effect->decode(buffer);
			map[std::move(identifier)] = std::move(effect);
		}

		return buffer;
	}
}
