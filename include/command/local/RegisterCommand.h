#pragma once

#include "command/local/LocalCommand.h"

namespace Game3 {
	struct RegisterCommand: LocalCommand {
		constexpr static const char *name = "reg";
		using LocalCommand::LocalCommand;
		void operator()(LocalClient &) override;
		std::string getName() const override { return std::string(name); }
	};
}
