#pragma once

#include "command/local/LocalCommand.h"

namespace Game3 {
	struct WikiCommand: LocalCommand {
		constexpr static const char *name = "wiki";
		using LocalCommand::LocalCommand;
		void operator()(LocalClient &) override;
		std::string getName() const override { return std::string(name); }
	};
}
