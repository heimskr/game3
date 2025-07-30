#include "entity/Player.h"
#include "entity/Speaker.h"
#include "dialogue/Dialogue.h"
#include "game/ClientGame.h"

namespace Game3 {
	DialogueNode::DialogueNode(DialogueGraph &parent, std::string name, std::vector<DialogueOption> options, TexturePtr faceOverride):
		parent(parent),
		name(std::move(name)),
		options(std::move(options)),
		faceOverride(std::move(faceOverride)) {}

	void DialogueNode::select() {
		auto self = shared_from_this();
		if (auto speaker = parent.getSpeaker()) {
			speaker->dialogueSelected(self);
		}
		parent.setActiveNode(std::move(self));
	}

	UString DialogueNode::getDisplay() {
		return display;
	}

	bool DialogueNode::render(UIContext &, const RendererContext &) {
		return false;
	}

	bool DialogueNode::keyPressed(uint32_t, Modifiers, bool) {
		return false;
	}

	DialogueGraph::DialogueGraph(std::shared_ptr<Player> player):
		player(std::move(player)) {}

	DialogueGraph::~DialogueGraph() = default;

	DialogueNodePtr DialogueGraph::addNode(std::string name, UString display, std::vector<DialogueOption> options, TexturePtr faceOverride) {
		auto node = std::make_shared<DialogueNode>(*this, name, std::move(options), std::move(faceOverride));
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
