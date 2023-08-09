#pragma once

#include "command/local/LocalCommand.h"

namespace Game3 {
	struct ChemicalCommand: LocalCommand {
		constexpr static const char *name = "chem";
		using LocalCommand::LocalCommand;
		void operator()(LocalClient &) override;
		std::string getName() const override { return std::string(name); }
	};
}
