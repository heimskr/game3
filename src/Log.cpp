#include "Log.h"
#include "util/Util.h"

namespace Game3 {
	Logger log;

	std::string Logger::getTimestamp() {
		return formatTime("%H:%M:%S");
	}
}
