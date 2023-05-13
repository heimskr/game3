#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "Log.h"
#include "net/Sock.h"
#include "net/NetError.h"
#include "net/ResolutionError.h"

namespace Game3 {
	int Sock::sockCount = 0;

	Sock::Sock(std::string_view hostname_, uint16_t port_):
	hostname(hostname_), port(port_) {
		struct addrinfo hints = {};
		std::memset(&hints, 0, sizeof(hints));
		hints.ai_family   = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		int status = getaddrinfo(hostname.c_str(), std::to_string(port).c_str(), &hints, &info);
		if (status != 0)
			throw ResolutionError(errno);
	}

	Sock::~Sock() {
		close();
		freeaddrinfo(info);
	}

	void Sock::connect(bool blocking) {
		netFD = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
		int status = ::connect(netFD, info->ai_addr, info->ai_addrlen);

		if (status != 0) {
			ERROR("connect(): " << strerror(errno));
			throw NetError(errno);
		}

		int flags = fcntl(netFD, F_GETFL, 0);
		if (flags == -1) {
			ERROR("fcntl (get): " << strerror(errno));
			throw NetError(errno);
		}

		flags = fcntl(netFD, F_SETFL, blocking? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK));
		if (flags == -1) {
			ERROR("fcntl (set): " << strerror(errno));
			throw NetError(errno);
		}

		int control_pipe[2];
		status = pipe(control_pipe);
		if (status != 0) {
			ERROR("pipe(): " << strerror(errno));
			throw NetError(errno);
		}

		controlRead  = control_pipe[0];
		controlWrite = control_pipe[1];

		FD_ZERO(&fds);
		FD_SET(netFD, &fds);
		FD_SET(controlRead, &fds);

		connected = true;
	}

	void Sock::close() {
		if (connected) {
			ControlMessage message = ControlMessage::Close;
			::write(controlWrite, &message, sizeof(message));
			connected = false;
		}
	}

	ssize_t Sock::send(const void *data, size_t bytes) {
		if (!connected)
			throw std::invalid_argument("Socket not connected");
		return ::send(netFD, data, bytes, 0);
	}

	ssize_t Sock::recv(void *data, size_t bytes) {
		if (!connected)
			throw std::invalid_argument("Socket not connected");

		fd_set fds_copy = fds;
		int status = select(FD_SETSIZE, &fds_copy, NULL, NULL, NULL);
		if (status < 0) {
			ERROR("select status: " << strerror(status));
			throw NetError(errno);
		}

		if (FD_ISSET(netFD, &fds_copy)) {
			ssize_t bytes_read = ::recv(netFD, data, bytes, 0);
			if (bytes_read == 0)
				close();

			return bytes_read;
		}

		if (FD_ISSET(controlRead, &fds_copy)) {
			ControlMessage message;
			status = ::read(controlRead, &message, 1);
			if (status < 0) {
				ERROR("control_fd status: " << strerror(status));
				throw NetError(errno);
			}

			if (message != ControlMessage::Close) {
				ERROR("Unknown control message: '" << static_cast<char>(message) << "'");
			}

			return 0;
		}

		SPAM("No file descriptor is ready.");
		return -1;
	}
}
