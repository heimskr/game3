#pragma once

#include "game/Forward.h"
#include "graphics/Forward.h"
#include "types/UString.h"
#include "ui/widget/Forward.h"

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Game3 {
	class Player;
	class Speaker;
	class UIContext;
	struct Modifiers;
	struct RendererContext;

	class DialogueGraph;
	using DialogueGraphPtr = std::shared_ptr<DialogueGraph>;

	struct DialogueOption {
		/** The text to display as the option. */
		UString display;
		/** The name of the dialogue node to continue to. */
		std::string nodeTarget;
	};

	using DialogueOptions = std::vector<DialogueOption>;

	class DialogueNode: public std::enable_shared_from_this<DialogueNode> {
		public:
			std::string name;
			UString display;
			std::vector<DialogueOption> options;
			TexturePtr faceOverride;

			DialogueNode(const DialogueGraphPtr &parent, std::string name, std::vector<DialogueOption> options = {}, TexturePtr faceOverride = {});

			virtual ~DialogueNode();

			void selectOption(size_t) const;

			DialogueGraphPtr getParent() const;
			virtual void select();
			virtual UString getDisplay();
			virtual WidgetPtr getWidget(UIContext &);

		protected:
			std::weak_ptr<DialogueGraph> weakParent;
			WidgetPtr cachedWidget;
	};

	using DialogueNodePtr = std::shared_ptr<DialogueNode>;

	class DialogueGraph: public std::enable_shared_from_this<DialogueGraph> {
		public:
			static DialogueGraphPtr create(std::shared_ptr<Player> player);

			virtual ~DialogueGraph();

			DialogueNodePtr addNode(std::string name, UString display, std::vector<DialogueOption> options = {}, TexturePtr faceOverride = {});
			DialogueNodePtr addNode(std::string name, UString display, std::vector<DialogueOption> options, const std::filesystem::path &texturePath);
			void addNode(DialogueNodePtr);
			ClientGamePtr getGame() const;

			virtual void selectNode(const std::string &name);
			virtual void close();

			virtual DialogueNodePtr getActiveNode() const { return activeNode; }
			virtual void setActiveNode(DialogueNodePtr);

			virtual std::shared_ptr<Speaker> getSpeaker() const { return speaker; }
			virtual void setSpeaker(std::shared_ptr<Speaker>);

			auto getStillOpen() const { return stillOpen; }

		private:
			DialogueGraph(std::shared_ptr<Player> player);

			std::shared_ptr<Speaker> speaker;
			std::shared_ptr<Player> player;
			std::unordered_map<std::string, DialogueNodePtr> nodes;
			DialogueNodePtr activeNode;
			bool stillOpen = true;
	};
}
