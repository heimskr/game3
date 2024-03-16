#pragma once

#include "game/ClientInventory.h"
#include "tileentity/EnergeticTileEntity.h"
#include "tileentity/FluidHoldingTileEntity.h"
#include "tileentity/InventoriedTileEntity.h"
#include "types/Types.h"
#include "ui/module/Module.h"
#include "ui/module/EnergyLevelModule.h"
#include "ui/module/FluidLevelsModule.h"
#include "ui/module/InventoryModule.h"
#include "util/Cast.h"

#include <any>
#include <memory>
#include <vector>

namespace Game3 {
	class Agent;
	class Microscope;
	class InventoryModule;
	class InventoryTab;

	template <Substance... S>
	class MultiModule: public Module {
		public:
			static Identifier ID() {
				const static std::map<Substance, char> substances{
					{Substance::Item,   'i'},
					{Substance::Fluid,  'f'},
					{Substance::Energy, 'e'},
				};

				std::string name = "module/multi_";
				for (Substance substance: {S...})
					name += substances.at(substance);

				return {"base", std::move(name)};
			}

			MultiModule(std::shared_ptr<ClientGame> game, const std::any &argument, ItemSlotParent *item_slot_parent = nullptr, InventoryModule::GmenuFn gmenu_fn = {}) {
				AgentPtr agent = std::any_cast<AgentPtr>(argument);

				header.set_text(agent->getName());
				vbox.append(header);

				for (Substance substance: {S...}) {
					switch (substance) {
						case Substance::Item: {
							auto inventoried = std::dynamic_pointer_cast<InventoriedTileEntity>(agent);
							assert(inventoried);
							for (size_t index = 0; index < inventoried->getInventoryCount(); ++index) {
								auto inventory = safeDynamicCast<ClientInventory>(inventoried->getInventory(index));
								submodules.push_back(std::make_shared<InventoryModule>(game, std::move(inventory), item_slot_parent, gmenu_fn));
							}
							break;
						}

						case Substance::Fluid: {
							assert(std::dynamic_pointer_cast<FluidHoldingTileEntity>(agent));
							submodules.push_back(std::make_shared<FluidLevelsModule>(game, argument, false));
							break;
						}

						case Substance::Energy: {
							assert(std::dynamic_pointer_cast<EnergeticTileEntity>(agent));
							submodules.push_back(std::make_shared<EnergyLevelModule>(game, argument, false));
							break;
						}

						default:
							throw std::invalid_argument(std::format("Invalid Substance: {}", int(substance)));
					}

					vbox.append(submodules.back()->getWidget());
				}
			}

			Identifier getID() const final {
				return ID();
			}

			Gtk::Widget & getWidget() final {
				return vbox;
			}

			void reset() final {
				for (const std::shared_ptr<Module> &submodule: submodules)
					submodule->reset();
			}

			void update() final {
				for (const std::shared_ptr<Module> &submodule: submodules)
					submodule->update();
			}

			void onResize(int size) final {
				for (const std::shared_ptr<Module> &submodule: submodules)
					submodule->onResize(size);
			}

			std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final {
				for (const std::shared_ptr<Module> &submodule: submodules)
					if (std::optional<Buffer> buffer = submodule->handleMessage(source, name, data))
						return buffer;
				return std::nullopt;
			}

			void setInventory(std::shared_ptr<ClientInventory> inventory) final {
				if (auto submodule = getPrimaryInventoryModule())
					submodule->setInventory(std::move(inventory));
			}

			std::shared_ptr<InventoryModule> getPrimaryInventoryModule() final {
				for (const std::shared_ptr<Module> &submodule: submodules)
					if (auto inventory_module = std::dynamic_pointer_cast<InventoryModule>(submodule))
						return inventory_module;
				return nullptr;
			}

		private:
			Gtk::Label header;
			std::vector<std::shared_ptr<Module>> submodules;
			Gtk::Box vbox{Gtk::Orientation::VERTICAL};

			void populate();
	};
}
