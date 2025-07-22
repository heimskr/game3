#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "types/UString.h"

namespace Game3 {
	class DialogueGraph;
	class Player;
	class Speaker;

	struct DialogueOption {
		/** The text to display as the option. */
		UString display;
		/** The name of the dialogue node to continue to. */
		std::string nodeTarget;
	};

	class DialogueNode: public std::enable_shared_from_this<DialogueNode> {
		public:
			DialogueGraph &parent;
			std::string name;
			UString display;
			std::vector<DialogueOption> options;

			DialogueNode(DialogueGraph &parent, std::string name, std::vector<DialogueOption> options = {});

			virtual ~DialogueNode() = default;

			virtual void select();
			virtual UString getDisplay();
	};

	using DialogueNodePtr = std::shared_ptr<DialogueNode>;

	class DialogueGraph {
		public:
			DialogueGraph(std::shared_ptr<Player> player);

			virtual ~DialogueGraph();

			DialogueNodePtr addNode(std::string name, UString display, std::vector<DialogueOption> options = {});

			virtual void selectNode(const std::string &name);
			virtual void close();

			virtual DialogueNodePtr getActiveNode() const { return activeNode; }
			virtual void setActiveNode(DialogueNodePtr);

			virtual std::shared_ptr<Speaker> getSpeaker() const { return speaker; }
			virtual void setSpeaker(std::shared_ptr<Speaker>);

			auto getStillOpen() const { return stillOpen; }

		private:
			std::shared_ptr<Speaker> speaker;
			std::shared_ptr<Player> player;
			std::unordered_map<std::string, DialogueNodePtr> nodes;
			DialogueNodePtr activeNode;
			bool stillOpen = true;
	};

	using DialogueGraphPtr = std::shared_ptr<DialogueGraph>;
}
