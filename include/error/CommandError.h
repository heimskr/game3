#pragma once

#include "error/Warning.h"

namespace Game3 {
	/** Thrown when the user inputs an invalid command. */
	struct CommandError: Warning {
		using Warning::Warning;
	};
}
