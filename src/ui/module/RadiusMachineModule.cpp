#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "packet/UpdateAgentFieldPacket.h"
#include "tileentity/EnergeticTileEntity.h"
#include "tileentity/FluidHoldingTileEntity.h"
#include "tileentity/InventoriedTileEntity.h"
#include "ui/module/RadiusMachineModule.h"
#include "ui/module/EnergyModule.h"
#include "ui/module/FluidsModule.h"
#include "ui/module/InventoryModule.h"
#include "ui/widget/Box.h"
#include "ui/widget/Label.h"
#include "ui/widget/Slider.h"
#include "ui/GameUI.h"
#include "ui/Window.h"
#include "util/Cast.h"
#include "util/ConstexprHash.h"

#include <cassert>

namespace Game3 {
	RadiusMachineModule::RadiusMachineModule(UIContext &ui, float selfScale, const ClientGamePtr &game, const std::any &argument):
		RadiusMachineModule(ui, selfScale, game, safeDynamicCast<TileEntity>(std::any_cast<AgentPtr>(argument))) {}

	RadiusMachineModule::RadiusMachineModule(UIContext &ui, float selfScale, const ClientGamePtr &game, TileEntityPtr machine):
		Module(ui, selfScale, game),
		machine(std::move(machine)) {}

	void RadiusMachineModule::init() {
		assert(machine != nullptr);

		ClientGamePtr game = getGame();

		if (auto inventoried = std::dynamic_pointer_cast<InventoriedTileEntity>(machine)) {
			inventoryModule = make<InventoryModule>(ui, selfScale, std::static_pointer_cast<ClientInventory>(inventoried->getInventory(0)));
		}

		if (std::dynamic_pointer_cast<EnergeticTileEntity>(machine)) {
			energyModule = make<EnergyModule>(ui, selfScale, game, std::static_pointer_cast<Agent>(machine), false);
		}

		if (auto fluidic = std::dynamic_pointer_cast<FluidHoldingTileEntity>(machine)) {
			fluidsModule = make<FluidsModule>(ui, selfScale, std::static_pointer_cast<HasFluids>(fluidic), false);
		}

		auto has_radius = safeDynamicCast<HasRadius>(machine);

		auto vbox = make<Box>(ui, selfScale, Orientation::Vertical);
		vbox->insertAtEnd(shared_from_this());

		auto header = make<Label>(ui, selfScale);
		header->setText(machine->getName());
		header->insertAtEnd(vbox);

		if (inventoryModule) {
			inventoryModule->insertAtEnd(vbox);
		}

		auto slider = make<Slider>(ui, selfScale);
		slider->setRange(1, 100);
		slider->setDisplayDigits(0);
		slider->setStep(1);
		slider->setValue(has_radius->getRadius());
		slider->onRelease.connect([this](Slider &, double value) {
			getGame()->send(make<UpdateAgentFieldPacket>(*machine, "radius"_fnv, static_cast<uint64_t>(value)));
		});
		slider->insertAtEnd(vbox);

		if (energyModule) {
			energyModule->insertAtEnd(vbox);
		}

		if (fluidsModule) {
			fluidsModule->insertAtEnd(vbox);
		}
	}

	void RadiusMachineModule::render(const RendererContext &renderers, float x, float y, float width, float height) {
		Module::render(renderers, x, y, width, height);
		firstChild->render(renderers, x, y, width, height);
	}

	SizeRequestMode RadiusMachineModule::getRequestMode() const {
		return firstChild->getRequestMode();
	}

	void RadiusMachineModule::measure(const RendererContext &renderers, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) {
		return firstChild->measure(renderers, orientation, for_width, for_height, minimum, natural);
	}

	std::optional<Buffer> RadiusMachineModule::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
		if (name == "TileEntityRemoved") {

			if (source && source->getGID() == machine->getGID()) {
				ClientGamePtr game = getGame();
				if (inventoryModule) {
					inventoryModule->handleMessage(source, name, data);
				}
				game->getWindow()->queue([](Window &window) {
					if (auto game_ui = window.uiContext.getUI<GameUI>()) {
						game_ui->removeModule();
					}
				});
			}

		} else if (name == "GetAgentGID") {

			return Buffer{Side::Client, machine->getGID()};

		} else if (name == "UpdateEnergy") {

			if (energyModule) {
				return energyModule->handleMessage(source, name, data);
			}

			return {};

		} else if (name == "UpdateFluids") {

			if (fluidsModule) {
				return fluidsModule->handleMessage(source, name, data);
			}

			return {};

		} else if (inventoryModule) {

			return inventoryModule->handleMessage(source, name, data);

		}

		return {};
	}

	void RadiusMachineModule::setInventory(std::shared_ptr<ClientInventory> inventory) {
		if (inventory->index == 0) {
			inventoryModule->setInventory(std::move(inventory));
		} else {
			throw std::invalid_argument("Can't set RadiusMachineModule inventory at index " + std::to_string(inventory->index));
		}
	}

	std::shared_ptr<InventoryModule> RadiusMachineModule::getPrimaryInventoryModule() {
		return inventoryModule;
	}
}
