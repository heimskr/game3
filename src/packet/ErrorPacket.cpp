#include "Log.h"
#include "packet/ErrorPacket.h"

namespace Game3 {
	void ErrorPacket::handle(ClientGame &) {
		ERROR("Error: " << error);
	}
}
