#pragma once

#include "command/local/LocalCommand.h"

namespace Game3 {
	struct ScaleCommand: LocalCommand {
		constexpr static const char *name = "scale";
		using LocalCommand::LocalCommand;
		void operator()(LocalClient &) override;
		std::string getName() const override { return std::string(name); }
	};
}
