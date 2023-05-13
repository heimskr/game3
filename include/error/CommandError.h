#pragma once

#include <stdexcept>

namespace Game3 {
	/** Thrown when the user inputs an invalid command. */
	struct CommandError: std::runtime_error {
		using std::runtime_error::runtime_error;
	};
}
