#pragma once

#include "net/Buffer.h"
#include "net/GenericClient.h"
#include "packet/Packet.h"

namespace Game3 {
	class LocalServer;

	/** Used by servers to represent a remote client. */
	class RemoteClient: public GenericClient {
		public:
			constexpr static size_t MAX_PACKET_SIZE = 1 << 24;

			LocalServer &server;

			RemoteClient() = delete;
			RemoteClient(LocalServer &server_, int id_, std::string_view ip_):
				GenericClient(id_, ip_), server(server_) {}

			~RemoteClient() override = default;

			void handleInput(std::string_view) override;
			void send(const Packet &);

			template <typename T>
			requires (!std::derived_from<T, Packet>)
			void send(const T &value);

		private:
			enum class State {Begin, Data};

			State state = State::Begin;

			std::vector<uint8_t> headerBytes;
			uint16_t packetType = 0;
			uint32_t packetSize = 0;




			Buffer buffer;
	};
}
