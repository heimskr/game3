#include "util/Promises.h"

namespace Game3 {
	PromiseRef<void> sleepFor(uint64_t milliseconds) {
		return sleepFor(std::chrono::milliseconds(milliseconds));
	}
}