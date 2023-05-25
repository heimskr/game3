#include <atomic>
#include <stdexcept>

#include "ui/Text.h"

namespace Game3 {
	static std::atomic_bool ready = false;

	void initText() {
		if (ready.exchange(true))
			return;
	}

	void drawText(const char *text) {
		initText();
	}
}
