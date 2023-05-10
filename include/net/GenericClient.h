#pragma once

#include <cstddef>
#include <string>

namespace Game3 {
	class Packet;

	struct GenericClient {
		int id = -1;
		std::string ip;
		/** If nonzero, don't read more than this many bytes at a time. The amount read will be subtracted from this. */
		size_t maxRead = 0;

		GenericClient() = delete;
		GenericClient(const GenericClient &) = delete;
		GenericClient(GenericClient &&) = delete;
		GenericClient(int id_, std::string_view ip_):
			id(id_), ip(ip_) {}

		virtual ~GenericClient() = default;

		GenericClient & operator=(const GenericClient &) = delete;
		GenericClient & operator=(GenericClient &&) = delete;

		virtual void handleInput(std::string_view) = 0;
		virtual void onMaxLineSizeExceeded() {}
	};
}
