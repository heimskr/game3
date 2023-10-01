#pragma once

#include "command/local/LocalCommand.h"

#include <csignal>

namespace Game3 {
	struct SegfaultCommand: LocalCommand {
		constexpr static const char *name = "segfault";
		using LocalCommand::LocalCommand;
		void operator()(LocalClient &) override { raise(SIGSEGV); }
		std::string getName() const override { return std::string(name); }
	};
}
