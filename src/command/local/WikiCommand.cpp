#include "command/local/WikiCommand.h"
#include "util/Util.h"

namespace Game3 {
	void WikiCommand::operator()(LocalClient &) {
		if (pieces.size() < 2) {
			throw CommandError("\"wiki\" command arguments: <query>");
		}

		// TODO: proper escaping
		openInBrowser(std::format("https://game3.gay/w/Special:Search?search={}", join(std::span(pieces).subspan(1), " ")));
	}
}
