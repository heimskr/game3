#include "dialogue/Dialogue.h"
#include "entity/Player.h"
#include "entity/Speaker.h"
#include "game/ClientGame.h"
#include "ui/widget/DialogueDisplay.h"

namespace Game3 {
	DialogueNode::DialogueNode(const DialogueGraphPtr &parent, std::string name, std::vector<DialogueOption> options, TexturePtr faceOverride):
		name(std::move(name)),
		options(std::move(options)),
		faceOverride(std::move(faceOverride)),
		weakParent(parent) {}

	DialogueNode::~DialogueNode() = default;

	void DialogueNode::selectOption(size_t index) const {
		getParent()->selectNode(options.at(index).nodeTarget);
	}

	DialogueGraphPtr DialogueNode::getParent() const {
		return weakParent.lock();
	}

	void DialogueNode::select() {
		auto self = shared_from_this();
		DialogueGraphPtr parent = getParent();

		if (auto speaker = parent->getSpeaker()) {
			speaker->dialogueSelected(self);
		}

		parent->setActiveNode(std::move(self));
	}

	UString DialogueNode::getDisplay() {
		return display;
	}

	WidgetPtr DialogueNode::getWidget(UIContext &ui) {
		if (!cachedWidget) {
			cachedWidget = make<DialogueDisplay>(ui, 1, shared_from_this());
		}

		return cachedWidget;
	}

	DialogueGraphPtr DialogueGraph::create(std::shared_ptr<Player> player) {
		return std::shared_ptr<DialogueGraph>(new DialogueGraph(std::move(player)));
	}

	DialogueGraph::DialogueGraph(std::shared_ptr<Player> player):
		player(std::move(player)) {}

	DialogueGraph::~DialogueGraph() = default;

	DialogueNodePtr DialogueGraph::addNode(std::string name, UString display, std::vector<DialogueOption> options, TexturePtr faceOverride) {
		auto node = std::make_shared<DialogueNode>(shared_from_this(), name, std::move(options), std::move(faceOverride));
		node->display = std::move(display);
		nodes[std::move(name)] = node;

		if (activeNode == nullptr || nodes.size() == 1) {
			activeNode = node;
		}

		return node;
	}

	void DialogueGraph::addNode(DialogueNodePtr node) {
		nodes[node->name] = node;

		if (activeNode == nullptr || nodes.size() == 1) {
			activeNode = node;
		}
	}

	DialogueNodePtr DialogueGraph::addNode(std::string name, UString display, std::vector<DialogueOption> options, const std::filesystem::path &texturePath) {
		return addNode(std::move(name), std::move(display), std::move(options), cacheTexture(texturePath));
	}

	ClientGamePtr DialogueGraph::getGame() const {
		return std::static_pointer_cast<ClientGame>(player->getGame());
	}

	void DialogueGraph::selectNode(const std::string &name) {
		if (name == "!exit") {
			close();
		} else if (auto iter = nodes.find(name); iter != nodes.end()) {
			setActiveNode(iter->second);
		} else {
			throw std::out_of_range(std::format("No dialogue node named \"{}\"", name));
		}
	}

	void DialogueGraph::close() {
		activeNode.reset();
		nodes.clear();
		stillOpen = false;
	}

	void DialogueGraph::setActiveNode(DialogueNodePtr node) {
		activeNode = std::move(node);
	}

	void DialogueGraph::setSpeaker(std::shared_ptr<Speaker> value) {
		speaker = std::move(value);
	}
}
