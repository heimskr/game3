#pragma once

#include <functional>
#include <memory>

#include "types/Types.h"
#include "packet/Packet.h"
#include "registry/Registerable.h"

namespace Game3 {
	class Game;

	class PacketFactory: public NumericRegisterable {
		private:
			std::function<std::shared_ptr<Packet>()> function;

		public:
			PacketFactory(PacketID, decltype(function));

			std::shared_ptr<Packet> operator()();

			template <typename T>
			static PacketFactory create(PacketID id = T::ID()) {
				return {id, [] {
					return std::make_shared<T>();
				}};
			}
	};
}
