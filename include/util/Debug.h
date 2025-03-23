#pragma once

#include <csignal>

namespace Game3 {
	inline void Break() {
#ifdef __MINGW32__
		asm volatile("int3"); // ???
#else
		raise( SIGTRAP)
#endif
	}
}
