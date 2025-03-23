#include "util/Log.h"
#include "util/Util.h"

namespace Game3::Logger {
	int level = 1;

	std::string getTimestamp() {
		return formatTime("%H:%M:%S");
	}
}
