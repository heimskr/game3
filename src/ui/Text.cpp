#include <atomic>
#include <memory>
#include <stdexcept>

#include "util/GL.h"

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
