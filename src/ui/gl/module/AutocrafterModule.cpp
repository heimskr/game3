#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "recipe/CraftingRecipe.h"
#include "tileentity/Autocrafter.h"
#include "ui/gl/module/AutocrafterModule.h"
#include "ui/gl/module/EnergyModule.h"
#include "ui/gl/module/InventoryModule.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/widget/Scroller.h"
#include "ui/gl/widget/TextInput.h"
#include "ui/Window.h"
#include "util/Cast.h"

#include <cassert>

namespace Game3 {
	AutocrafterModule::AutocrafterModule(UIContext &ui, const ClientGamePtr &game, const std::any &argument):
		AutocrafterModule(ui, game, safeDynamicCast<Autocrafter>(std::any_cast<AgentPtr>(argument))) {}

	AutocrafterModule::AutocrafterModule(UIContext &ui, const ClientGamePtr &game, std::shared_ptr<Autocrafter> autocrafter):
		Module(ui, game),
		autocrafter(std::move(autocrafter)),
		inventoryModule(std::make_shared<InventoryModule>(ui, std::static_pointer_cast<ClientInventory>(this->autocrafter->getInventory(0)))),
		stationInventoryModule(std::make_shared<InventoryModule>(ui, std::static_pointer_cast<ClientInventory>(this->autocrafter->getInventory(1)))),
		energyModule(std::make_shared<EnergyModule>(ui, game, std::static_pointer_cast<Agent>(this->autocrafter), false)) {}

	void AutocrafterModule::init() {
		assert(autocrafter);

		ClientGamePtr game = getGame();

		auto vbox = std::make_shared<Box>(ui, selfScale, Orientation::Vertical);
		vbox->insertAtEnd(shared_from_this());

		auto header = std::make_shared<Label>(ui, selfScale);
		header->setText(autocrafter->getName());
		header->insertAtEnd(vbox);

		identifierInput = std::make_shared<TextInput>(ui, selfScale);
		identifierInput->setText(autocrafter->getTarget().copyBase().str());
		identifierInput->onSubmit.connect([this](TextInput &, const UString &text) {
			setTarget(text.raw());
		});
		identifierInput->onAcceptSuggestion.connect([this](TextInput &input, const UString &target) {
			setTarget(target.raw());
			input.hideDropdown();
		});
		std::set<UString> item_names;
		for (const auto &recipe: game->registry<CraftingRecipeRegistry>())
			for (const ItemStackPtr &stack: recipe->output)
				item_names.insert(UString(stack->item->identifier.str()));
		identifierInput->setSuggestions(std::vector(std::move_iterator(item_names.begin()), std::move_iterator(item_names.end())));
		identifierInput->insertAtEnd(vbox);

		stationInventoryModule->insertAtEnd(vbox);
		inventoryModule->insertAtEnd(vbox);
		energyModule->insertAtEnd(vbox);

		stationInventoryModule->init();
		inventoryModule->init();
		energyModule->init();
	}

	void AutocrafterModule::render(const RendererContext &renderers, float x, float y, float width, float height) {
		Module::render(renderers, x, y, width, height);
		firstChild->render(renderers, x, y, width, height);
	}

	SizeRequestMode AutocrafterModule::getRequestMode() const {
		return firstChild->getRequestMode();
	}

	void AutocrafterModule::measure(const RendererContext &renderers, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) {
		return firstChild->measure(renderers, orientation, for_width, for_height, minimum, natural);
	}

	std::optional<Buffer> AutocrafterModule::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
		if (name == "TargetSet") {

			auto *buffer = std::any_cast<Buffer>(&data);
			assert(buffer != nullptr);
			const bool success = buffer->take<bool>();
			Identifier id = buffer->take<Identifier>();

			if (success) {
				identifierInput->setText(id.str());
			} else {
				identifierInput->setText({});
			}

			autocrafter->setTarget(std::move(id));

		} else if (name == "TileEntityRemoved") {

			if (source && source->getGID() == autocrafter->getGID()) {
				ClientGamePtr game = getGame();
				stationInventoryModule->handleMessage(source, name, data);
				inventoryModule->handleMessage(source, name, data);
				game->getWindow()->queue([](Window &window) {
					window.removeModule();
				});
			}

		} else if (name == "GetAgentGID") {

			return Buffer{Side::Client, autocrafter->getGID()};

		} else if (name == "UpdateEnergy") {

			return energyModule->handleMessage(source, name, data);

		} else {
			return inventoryModule->handleMessage(source, name, data);
		}

		return {};
	}

	void AutocrafterModule::setInventory(std::shared_ptr<ClientInventory> inventory) {
		if (inventory->index == 0) {
			inventoryModule->setInventory(std::move(inventory));
		} else if (inventory->index == 1) {
			stationInventoryModule->setInventory(std::move(inventory));
		} else {
			throw std::invalid_argument("Can't set AutocrafterModule inventory at index " + std::to_string(inventory->index));
		}
	}

	std::shared_ptr<InventoryModule> AutocrafterModule::getPrimaryInventoryModule() {
		return inventoryModule;
	}

	void AutocrafterModule::setTarget(const std::string &target) {
		if (autocrafter) {
			ClientGamePtr game = getGame();
			game->getPlayer()->sendMessage(autocrafter, "SetTarget", target);
		}
	}
}
