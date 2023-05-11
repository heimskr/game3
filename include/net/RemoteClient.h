#pragma once

#include "net/GenericClient.h"
#include "packet/Packet.h"

namespace Game3 {
	class LocalServer;

	/** Used by servers to represent a remote client. */
	class RemoteClient: public GenericClient {
		public:
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
	};
}
