#include "net/GenericClient.h"
#include "net/Server.h"

namespace Game3 {
	void GenericClient::startBuffering() {
		++buffer;
	}

	void GenericClient::flushBuffer(bool force) {
		if (!force && !buffer.active())
			return;
		auto lock = buffer.lock();
		server.send(*this, std::string_view(buffer.bytes.data(), buffer.bytes.size()), true);
		buffer.bytes.clear();
	}

	void GenericClient::stopBuffering() {
		if (!(--buffer).active())
			flushBuffer(true);
	}

	bool GenericClient::isBuffering() const {
		return buffer.active();
	}
}
