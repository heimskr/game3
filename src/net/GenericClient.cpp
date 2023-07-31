#include "Log.h"
#include "net/GenericClient.h"
#include "net/Server.h"

namespace Game3 {
	void GenericClient::startBuffering() {
		++sendBuffer;
	}

	void GenericClient::flushBuffer(bool force) {
		if (!force && !sendBuffer.active())
			return;
		std::vector<char> moved_buffer;
		{
			auto buffer_lock = sendBuffer.uniqueLock();
			moved_buffer = std::move(sendBuffer.bytes);
		}
		std::unique_lock network_lock(networkMutex);
		server.send(*this, std::string_view(moved_buffer.data(), moved_buffer.size()), true);
	}

	void GenericClient::stopBuffering() {
		if (!(--sendBuffer).active())
			flushBuffer(true);
	}

	bool GenericClient::isBuffering() const {
		return sendBuffer.active();
	}
}
