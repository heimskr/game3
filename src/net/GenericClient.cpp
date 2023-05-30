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
		auto buffer_lock = sendBuffer.lock();
		{
			std::unique_lock network_lock(networkMutex);
			server.send(*this, std::string_view(sendBuffer.bytes.data(), sendBuffer.bytes.size()), true);
		}
		sendBuffer.bytes.clear();
	}

	void GenericClient::stopBuffering() {
		if (!(--sendBuffer).active())
			flushBuffer(true);
	}

	bool GenericClient::isBuffering() const {
		return sendBuffer.active();
	}
}
