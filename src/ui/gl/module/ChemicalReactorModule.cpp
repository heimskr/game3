#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "tileentity/ChemicalReactor.h"
#include "ui/gl/module/ChemicalReactorModule.h"
#include "ui/gl/module/EnergyModule.h"
#include "ui/gl/module/InventoryModule.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/widget/Label.h"
#include "ui/gl/widget/TextInput.h"
#include "ui/gl/UIContext.h"
#include "ui/Window.h"
#include "util/Cast.h"

namespace Game3 {
	namespace {
		constexpr Color VALID_FORMULA_TEXT_COLOR{"#00ff00"};
		constexpr Color INVALID_FORMULA_TEXT_COLOR{"#ff0000"};
	}

	ChemicalReactorModule::ChemicalReactorModule(UIContext &ui, const ClientGamePtr &game, const std::any &argument):
		ChemicalReactorModule(ui, game, safeDynamicCast<ChemicalReactor>(std::any_cast<AgentPtr>(argument))) {}

	ChemicalReactorModule::ChemicalReactorModule(UIContext &ui, const ClientGamePtr &game, std::shared_ptr<ChemicalReactor> reactor):
		Module(ui, game), reactor(std::move(reactor)) {}

	void ChemicalReactorModule::init() {
		assert(reactor);

		ClientGamePtr game = getGame();

		vbox = std::make_shared<Box>(ui, scale, Orientation::Vertical);
		vbox->insertAtEnd(shared_from_this());

		header = std::make_shared<Label>(ui, scale);
		header->setText(reactor->getName());
		vbox->append(header);

		formulaInput = std::make_shared<TextInput>(ui, scale);
		formulaInput->setText(reactor->getEquation());
		formulaInput->onSubmit.connect([this](TextInput &, const UString &equation) {
			setEquation(equation);
		});
		formulaInput->onChange.connect([](TextInput &input, const UString &) {
			input.setTextColor();
		});
		vbox->append(formulaInput);

		inventoryModule = std::make_shared<InventoryModule>(ui, std::static_pointer_cast<ClientInventory>(reactor->getInventory(0)));
		inventoryModule->init();
		vbox->append(inventoryModule);

		energyModule = std::make_shared<EnergyModule>(ui, game, std::static_pointer_cast<Agent>(reactor), false);
		energyModule->init();
		vbox->append(energyModule);
	}

	void ChemicalReactorModule::render(const RendererContext &renderers, float x, float y, float width, float height) {
		Module::render(renderers, x, y, width, height);
		firstChild->render(renderers, x, y, width, height);
	}

	SizeRequestMode ChemicalReactorModule::getRequestMode() const {
		return firstChild->getRequestMode();
	}

	void ChemicalReactorModule::measure(const RendererContext &renderers, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) {
		return firstChild->measure(renderers, orientation, for_width, for_height, minimum, natural);
	}

	std::optional<Buffer> ChemicalReactorModule::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
		if (name == "EquationSet") {

			auto *buffer = std::any_cast<Buffer>(&data);
			assert(buffer != nullptr);
			const bool success = buffer->take<bool>();
			if (success) {
				formulaInput->setTextColor(VALID_FORMULA_TEXT_COLOR);
			} else {
				formulaInput->setTextColor(INVALID_FORMULA_TEXT_COLOR);
			}

		} else if (name == "TileEntityRemoved") {

			if (source && source->getGID() == reactor->getGID()) {
				inventoryModule->handleMessage(source, name, data);
				energyModule->handleMessage(source, name, data);
				getGame()->getWindow()->queue([](Window &window) {
					window.removeModule();
				});
			}

		} else if (name == "GetAgentGID") {

			return Buffer{Side::Client, reactor->getGID()};

		} else {

			return inventoryModule->handleMessage(source, name, data);

		}

		return {};
	}

	void ChemicalReactorModule::setInventory(std::shared_ptr<ClientInventory> inventory) {
		if (inventory->index == 0) {
			inventoryModule->setInventory(std::move(inventory));
		} else {
			throw std::invalid_argument("Can't set ChemicalReactorModule inventory at index " + std::to_string(inventory->index));
		}
	}

	void ChemicalReactorModule::setEquation(const std::string &equation) {
		if (reactor) {
			getGame()->getPlayer()->sendMessage(reactor, "SetEquation", equation);
		}
	}
}