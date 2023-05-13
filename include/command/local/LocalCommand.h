#pragma once

#include <string>
#include <vector>

#include "error/CommandError.h"

namespace Game3 {
	class LocalClient;

	struct LocalCommand {
		std::vector<std::string> pieces;

		LocalCommand() = default;
		LocalCommand(std::vector<std::string>);
		LocalCommand(std::string_view);

		virtual ~LocalCommand() = default;

		virtual void operator()(LocalClient &) = 0;
		virtual std::string getName() const = 0;
	};
}
