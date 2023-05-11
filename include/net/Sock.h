#pragma once

#include <netdb.h>
#include <string>

namespace Game3 {
	class Sock {
		friend class SocketBuffer;

		public:
			const std::string hostname;
			uint16_t port;

			Sock(std::string_view hostname_, uint16_t port_);

			Sock() = delete;
			Sock(const Sock &) = delete;
			Sock & operator=(const Sock &) = delete;

			virtual ~Sock();

			/** Connects to the socket. */
			virtual void connect();

			/** Closes the socket. */
			virtual void close();

			/** Sends a given number of bytes from a buffer through the socket and returns the number of bytes sent. */
			virtual ssize_t send(const void *, size_t);

			/** Reads a given number of bytes into a buffer from the socket and returns the number of bytes read. */
			virtual ssize_t recv(void *, size_t);

			inline bool isConnected() const { return connected; }

		protected:
			static int sockCount;
			struct addrinfo *info;
			int netFD = -1, controlRead = -1, controlWrite = -1;
			bool connected = false;
			fd_set fds = {0};

			enum class ControlMessage: char {Close='C'};
	};
}
