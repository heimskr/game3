#ifndef __MINGW32__
#define BACKWARD_HAS_BFD 1
#define BACKWARD_HAS_LIBUNWIND 1
#else
#define BACKWARD_SYSTEM_WINDOWS
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunqualified-std-cast-call"
#include "backward.hpp"
#pragma GCC diagnostic pop

namespace backward {
	backward::SignalHandling sh;
}
