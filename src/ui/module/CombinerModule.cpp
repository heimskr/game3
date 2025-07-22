#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "recipe/CombinerRecipe.h"
#include "tileentity/Combiner.h"
#include "ui/module/CombinerModule.h"
#include "ui/widget/Box.h"
#include "ui/widget/Label.h"
#include "ui/widget/TextInput.h"
#include "util/Cast.h"
#include "ui/GameUI.h"
#include "ui/Window.h"

namespace Game3 {
	CombinerModule::CombinerModule(UIContext &ui, float selfScale, const ClientGamePtr &game, const std::any &argument):
		CombinerModule(ui, selfScale, game, safeDynamicCast<Combiner>(std::any_cast<AgentPtr>(argument))) {}

	CombinerModule::CombinerModule(UIContext &ui, float selfScale, const ClientGamePtr &game, std::shared_ptr<Combiner> combiner):
		Module(ui, selfScale, game),
		combiner(std::move(combiner)) {}

	void CombinerModule::init() {
		ClientGamePtr game = getGame();

		vbox = std::make_shared<Box>(ui, selfScale, Orientation::Vertical, 5, 0, Color{});
		vbox->insertAtEnd(shared_from_this());

		auto header = std::make_shared<Label>(ui, selfScale);
		header->setText(combiner->getName());
		header->setHorizontalAlignment(Alignment::Center);
		vbox->append(header);

		std::set<Identifier> item_names;
		for (const auto &[id, recipe]: game->registry<CombinerRecipeRegistry>()) {
			item_names.insert(id);
		}

		std::vector<UString> suggestions;
		for (const Identifier &item_name: item_names) {
			suggestions.emplace_back(item_name.str());
		}

		textInput = std::make_shared<TextInput>(ui, selfScale);
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

		multiModule = std::make_shared<MultiModule<Substance::Item, Substance::Energy>>(ui, selfScale, game, std::static_pointer_cast<Agent>(combiner));
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
				game->getWindow()->queue([](Window &window) {
					if (auto game_ui = window.uiContext.getUI<GameUI>()) {
						game_ui->removeModule();
					}
				});
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

	std::shared_ptr<InventoryModule> CombinerModule::getPrimaryInventoryModule() {
		return multiModule->getPrimaryInventoryModule();
	}

	void CombinerModule::setTarget(const std::string &target) {
		if (combiner) {
			getGame()->getPlayer()->sendMessage(combiner, "SetTarget", target);
		}
	}
}
