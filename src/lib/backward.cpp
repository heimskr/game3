#ifndef __MINGW32__
#define BACKWARD_HAS_BFD 1
#define BACKWARD_HAS_LIBUNWIND 1
#else
#define BACKWARD_SYSTEM_WINDOWS
#endif

#include "backward.hpp"

namespace backward {
	backward::SignalHandling sh;
}
