#include "Options.h"

#ifndef HIDE_SOCK
#include "Log.h"
#include "net/DisconnectedError.h"
#include "net/NetError.h"
#include "net/ResolutionError.h"
#include "net/Sock.h"

#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <optional>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

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
		if (connected) {
			ControlMessage message = ControlMessage::Close;
			if (-1 == ::write(controlWrite, &message, sizeof(message)))
				WARN("Couldn't write control message to Sock pipe");
			::close(netFD);
			connected = false;
		}

		freeaddrinfo(info);
	}

	void Sock::connect(bool blocking) {
		netFD = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
		int status = ::connect(netFD, info->ai_addr, info->ai_addrlen);

		if (status != 0) {
			ERROR("connect(): {}", strerror(errno));
			throw NetError(errno);
		}

		int flags = fcntl(netFD, F_GETFL, 0);
		if (flags == -1) {
			ERROR("fcntl (get): {}", strerror(errno));
			throw NetError(errno);
		}

		flags = fcntl(netFD, F_SETFL, blocking? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK));
		if (flags == -1) {
			ERROR("fcntl (set): {}", strerror(errno));
			throw NetError(errno);
		}

		int control_pipe[2];
		status = pipe(control_pipe);
		if (status != 0) {
			ERROR("pipe(): {}", strerror(errno));
			throw NetError(errno);
		}

		controlRead  = control_pipe[0];
		controlWrite = control_pipe[1];

		FD_ZERO(&fds);
		FD_SET(netFD, &fds);
		FD_SET(controlRead, &fds);

		connected = true;
	}

	void Sock::close(bool force) {
		if (connected || force) {
			ControlMessage message = ControlMessage::Close;

			if (-1 == ::write(controlWrite, &message, sizeof(message)))
				WARN("Couldn't write to control pipe");

			if (force)
				::close(netFD);
		} else
			WARN("Can't close: not connected");
	}

	ssize_t Sock::send(const void *data, size_t bytes, bool force) {
		if (!connected)
			throw DisconnectedError("Socket not connected");
		if (force || !buffering)
			return ::send(netFD, data, bytes, 0);
		addToBuffer(data, bytes);
		return static_cast<ssize_t>(bytes);
	}

	ssize_t Sock::recv(void *data, size_t bytes) {
		if (!connected)
			throw DisconnectedError("Socket not connected");

		fd_set fds_copy = fds;
		timeval timeout {.tv_sec = 0, .tv_usec = 100};
		int status = select(FD_SETSIZE, &fds_copy, nullptr, nullptr, &timeout);
		if (status < 0) {
			ERROR("select status: {}", strerror(status));
			throw NetError(errno);
		}

		if (FD_ISSET(netFD, &fds_copy)) {
			ssize_t bytes_read = ::recv(netFD, data, bytes, 0);
			if (bytes_read == 0)
				close(false);

			return bytes_read;
		}

		if (FD_ISSET(controlRead, &fds_copy)) {
			ControlMessage message;
			status = ::read(controlRead, &message, 1);
			if (status < 0) {
				ERROR("control_fd status: {}", strerror(status));
				throw NetError(errno);
			}

			if (message != ControlMessage::Close) {
				ERROR("Unknown control message: '{}'", static_cast<char>(message));
			}

			::close(netFD);
			return 0;
		}

		SPAM("No file descriptor is ready.");
		return -1;
	}

	void Sock::startBuffering() {
		buffering = true;
	}

	void Sock::flushBuffer(bool do_lock) {
		std::optional<std::unique_lock<std::shared_mutex>> lock;

		if (do_lock)
			lock.emplace(bufferMutex);

		if (buffer.empty())
			return;

		send(buffer.data(), buffer.size(), true);
		buffer.clear();
	}

	void Sock::stopBuffering() {
		if (buffering.exchange(false))
			flushBuffer(true);
	}

	void Sock::addToBuffer(const void *data, size_t bytes) {
		const auto *char_data = reinterpret_cast<const char *>(data);
		std::unique_lock lock(bufferMutex);
		buffer.insert(buffer.end(), char_data, char_data + bytes);
	}
}
#endif
