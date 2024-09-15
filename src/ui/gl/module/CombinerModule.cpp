#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "recipe/CombinerRecipe.h"
#include "tileentity/Combiner.h"
#include "ui/gl/module/CombinerModule.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/widget/TextInput.h"
#include "util/Cast.h"
#include "ui/MainWindow.h"

namespace Game3 {
	CombinerModule::CombinerModule(UIContext &ui, const ClientGamePtr &game, const std::any &argument):
		CombinerModule(ui, game, safeDynamicCast<Combiner>(std::any_cast<AgentPtr>(argument))) {}

	CombinerModule::CombinerModule(UIContext &ui, const ClientGamePtr &game, std::shared_ptr<Combiner> combiner):
		Module(ui, game), combiner(std::move(combiner)) {}

	void CombinerModule::init() {
		ClientGamePtr game = getGame();

		vbox = std::make_shared<Box>(ui, scale, Orientation::Vertical, 5, 0, Color{});
		vbox->insertAtEnd(shared_from_this());

		auto header = std::make_shared<Label>(ui, scale);
		header->setText(combiner->getName());
		header->setHorizontalAlignment(Alignment::Middle);
		vbox->append(header);

		std::set<Identifier> item_names;
		for (const auto &[id, recipe]: game->registry<CombinerRecipeRegistry>())
			item_names.insert(id);

		std::vector<UString> suggestions;
		for (const Identifier &item_name: item_names)
			suggestions.emplace_back(item_name.str());

		textInput = std::make_shared<TextInput>(ui, scale);
		textInput->setText(combiner->getTarget().copyBase().str());
		textInput->setHorizontalExpand(true);
		textInput->setSuggestions(std::move(suggestions));
		textInput->onSubmit.connect([this](TextInput &, const UString &text) {
			setTarget(text.raw());
		});
		textInput->onAcceptSuggestion.connect([this](TextInput &, const UString &suggestion) {
			setTarget(suggestion.raw());
		});
		vbox->append(textInput);

		multiModule = std::make_shared<MultiModule<Substance::Item, Substance::Energy>>(ui, game, std::static_pointer_cast<Agent>(combiner));
		multiModule->init();
		vbox->append(multiModule);
	}

	void CombinerModule::render(const RendererContext &renderers, float x, float y, float width, float height) {
		Module::render(renderers, x, y, width, height);
		firstChild->render(renderers, x, y, width, height);
	}

	SizeRequestMode CombinerModule::getRequestMode() const {
		return firstChild->getRequestMode();
	}

	void CombinerModule::measure(const RendererContext &renderers, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) {
		firstChild->measure(renderers, orientation, for_width, for_height, minimum, natural);
	}

	std::optional<Buffer> CombinerModule::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
		if (name == "TargetSet") {

			auto *buffer = std::any_cast<Buffer>(&data);
			assert(buffer != nullptr);
			const bool success = buffer->take<bool>();
			const Identifier id = buffer->take<Identifier>();

			if (success) {
				textInput->setText(id.str());
			} else {
				textInput->setText({});
			}

		} else if (name == "TileEntityRemoved") {

			if (source && source->getGID() == combiner->getGID()) {
				ClientGamePtr game = getGame();
				multiModule->handleMessage(source, name, data);
				MainWindow &window = game->getWindow();
				window.queue([&window] { window.removeModule(); });
			}

		} else if (name == "GetAgentGID") {

			return Buffer{Side::Client, combiner->getGID()};

		} else {
			return multiModule->handleMessage(source, name, data);
		}

		return {};
	}

	void CombinerModule::setInventory(std::shared_ptr<ClientInventory> inventory) {
		multiModule->setInventory(std::move(inventory));
	}

	void CombinerModule::setTarget(const std::string &target) {
		if (combiner) {
			getGame()->getPlayer()->sendMessage(combiner, "SetTarget", target);
		}
	}
}
