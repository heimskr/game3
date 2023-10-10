#pragma once

#include "net/SendBuffer.h"

#include <cstddef>
#include <memory>
#include <string>

#include <asio.hpp>
#include <asio/ssl.hpp>

namespace Game3 {
	class Packet;
	class Server;

	class GenericClient {
		public:
			Server &server;
			int id = -1;
			std::string ip;
			SendBuffer sendBuffer;
			std::mutex networkMutex;
			asio::ssl::stream<asio::ip::tcp::socket> socket;
			asio::io_context::strand strand;

			GenericClient() = delete;
			GenericClient(const GenericClient &) = delete;
			GenericClient(GenericClient &&) = delete;
			GenericClient(Server &server_, std::string_view ip_, int id_, asio::ip::tcp::socket &&socket_);

			virtual ~GenericClient() = default;

			GenericClient & operator=(const GenericClient &) = delete;
			GenericClient & operator=(GenericClient &&) = delete;

			void start();

			virtual void handleInput(std::string_view) = 0;
			virtual void onMaxLineSizeExceeded() {}

			virtual void removeSelf() = 0;

		private:
			size_t bufferSize;
			std::unique_ptr<char[]> buffer;

			void doHandshake();
			void doRead();
	};
}
