#pragma once

#include <iostream>
#include <memory>

#include "util/Math.h"

namespace Game3 {
	class ClientGame;
	class Packet;
	class Sock;
	class SocketBuffer;

	class LocalClient {
		public:
			LocalClient() = default;

			void connect(std::string_view hostname, uint16_t port);
			void send(const Packet &);
			bool isConnected() const;

		private:
			std::shared_ptr<ClientGame> game;
			std::shared_ptr<Sock> sock;
			std::shared_ptr<SocketBuffer> buffer;
			std::shared_ptr<std::iostream> stream;

			template <typename T>
			void sendRaw(T value) {
				T little = toLittle(value);
				*stream << std::string_view(reinterpret_cast<const char *>(&little), sizeof(T));
			}
	};
}
