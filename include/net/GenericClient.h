#pragma once

#include <cstddef>
#include <string>

#include "net/SendBuffer.h"

struct bufferevent;

namespace Game3 {
	class Packet;
	class Server;

	struct GenericClient {
		Server &server;
		int id = -1;
		int descriptor = -1;
		std::string ip;
		/** If nonzero, don't read more than this many bytes at a time. The amount read will be subtracted from this. */
		size_t maxRead = 0;
		SendBuffer buffer;
		bufferevent *event = nullptr;

		GenericClient() = delete;
		GenericClient(const GenericClient &) = delete;
		GenericClient(GenericClient &&) = delete;
		GenericClient(Server &server_, int id_, int descriptor_, std::string_view ip_, bufferevent *event_):
			server(server_), id(id_), descriptor(descriptor_), ip(ip_), event(event_) {}

		virtual ~GenericClient() = default;

		GenericClient & operator=(const GenericClient &) = delete;
		GenericClient & operator=(GenericClient &&) = delete;

		virtual void handleInput(std::string_view) = 0;
		virtual void onMaxLineSizeExceeded() {}

		void startBuffering();
		void flushBuffer(bool force = false);
		void stopBuffering();
		bool isBuffering() const;
	};
}
