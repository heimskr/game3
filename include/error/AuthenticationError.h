#pragma once

#include "error/Warning.h"

namespace Game3 {
	struct AuthenticationError: Warning {
		using Warning::Warning;
	};
}
