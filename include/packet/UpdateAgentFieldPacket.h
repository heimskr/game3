#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"
#include "statuseffect/StatusEffectMap.h"

namespace Game3 {
	class Entity;

	struct UpdateAgentFieldPacket: Packet {
		static PacketID ID() { return 71; }

		GlobalID globalID = -1;
		uint32_t fieldNameHash = -1;
		Buffer fieldValue{Side::Client};

		UpdateAgentFieldPacket() = default;

		template <typename T>
		UpdateAgentFieldPacket(Agent &agent, uint32_t fieldNameHash, T &&value):
			globalID(agent.globalID),
			fieldNameHash(fieldNameHash) {
				fieldValue << std::forward<T>(value);
			}

		PacketID getID() const final { return ID(); }

		void encode(Game &, Buffer &) const override;
		void decode(Game &, BasicBuffer &)       override;

		void handle(const std::shared_ptr<ServerGame> &, GenericClient &) override;
		void handle(const std::shared_ptr<ClientGame> &) final;
	};
}
