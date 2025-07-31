#pragma once

#include <memory>

namespace Game3 {
	class DialogueNode;
	using DialogueNodePtr = std::shared_ptr<DialogueNode>;

	class DialogueGraph;
	using DialogueGraphPtr = std::shared_ptr<DialogueGraph>;
}
