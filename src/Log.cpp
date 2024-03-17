#include "Log.h"
#include "util/Util.h"

namespace Game3::Logger {
	std::mutex mutex;
	int level = 1;

	std::string getTimestamp() {
		return formatTime("%H:%M:%S");
	}
}
