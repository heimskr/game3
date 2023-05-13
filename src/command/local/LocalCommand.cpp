#include "command/local/LocalCommand.h"
#include "util/Util.h"

namespace Game3 {
	LocalCommand::LocalCommand(std::vector<std::string> pieces_):
		pieces(std::move(pieces_)) {}

	LocalCommand::LocalCommand(std::string_view command):
		pieces(split<std::string>(command, " ", false)) {}
}
