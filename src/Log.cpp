#include "Log.h"
#include "util/Util.h"

namespace Game3::Logger {
	std::mutex mutex;

	std::string getTimestamp() {
		return formatTime("%H:%M:%S");
	}
}
