#pragma once

#include "game/ClientInventory.h"
#include "tileentity/EnergeticTileEntity.h"
#include "tileentity/FluidHoldingTileEntity.h"
#include "tileentity/InventoriedTileEntity.h"
#include "types/Types.h"
#include "ui/gl/module/EnergyModule.h"
#include "ui/gl/module/FluidsModule.h"
#include "ui/gl/module/InventoryModule.h"
#include "ui/gl/module/Module.h"
#include "ui/gl/widget/Box.h"
#include "ui/gl/UIContext.h"
#include "util/Cast.h"

#include <any>
#include <map>
#include <string>

namespace Game3 {
	template <Substance... S>
	class MultiModule: public Module {
		public:
			static Identifier ID() {
				return {"base", "module/multi/" + getSuffix()};
			}

			static std::string getSuffix() {
				const static std::map<Substance, char> substances{
					{Substance::Item,   'i'},
					{Substance::Fluid,  'f'},
					{Substance::Energy, 'e'},
				};

				std::string suffix;
				for (Substance substance: {S...}) {
					suffix += substances.at(substance);
				}

				return suffix;
			}

			MultiModule(UIContext &ui, float selfScale, const ClientGamePtr &game, const std::any &argument):
				MultiModule(ui, selfScale, game, std::any_cast<AgentPtr>(argument)) {}

			MultiModule(UIContext &ui, float selfScale, const ClientGamePtr &game, const AgentPtr &agent):
				Module(ui, selfScale, game),
				agent(std::move(agent)) {}

			Identifier getID() const final {
				return ID();
			}

			void init() final {
				ClientGamePtr game = getGame();

				for (Substance substance: {S...}) {
					switch (substance) {
						case Substance::Item: {
							auto inventoried = std::dynamic_pointer_cast<InventoriedTileEntity>(agent);
							assert(inventoried);
							for (size_t i = 0; i < inventoried->getInventoryCount(); ++i) {
								auto inventory = safeDynamicCast<ClientInventory>(inventoried->getInventory(i));
								auto inventory_module = std::make_shared<InventoryModule>(ui, selfScale, std::move(inventory));
								submodules.emplace_back(inventory_module);
								if (firstInventoryModule == nullptr) {
									firstInventoryModule = inventory_module;
								}
							}
							break;
						}

						case Substance::Fluid: {
							assert(std::dynamic_pointer_cast<FluidHoldingTileEntity>(agent));
							submodules.emplace_back(std::make_shared<FluidsModule>(ui, selfScale, agent, false));
							break;
						}

						case Substance::Energy: {
							submodules.emplace_back(std::make_shared<EnergyModule>(ui, selfScale, agent, false));
							break;
						}

						default:
							throw std::invalid_argument(std::format("Invalid Substance: {}", static_cast<int>(substance)));
					}
				}

				box = std::make_shared<Box>(ui, selfScale, Orientation::Vertical);
				box->setHorizontalExpand(true);
				box->insertAtEnd(shared_from_this());

				for (const ModulePtr &submodule: submodules) {
					submodule->insertAtEnd(box);
					submodule->init();
				}
			}

			void reset() final {
				for (const ModulePtr &submodule: submodules) {
					submodule->reset();
				}
			}

			void update() final {
				for (const ModulePtr &submodule: submodules) {
					submodule->update();
				}
			}

			std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final {
				for (const ModulePtr &submodule: submodules) {
					if (std::optional<Buffer> buffer = submodule->handleMessage(source, name, data)) {
						return buffer;
					}
				}

				return std::nullopt;
			}

			std::shared_ptr<InventoryModule> getPrimaryInventoryModule() final {
				return firstInventoryModule;
			}

			void render(const RendererContext &renderers, float x, float y, float width, float height) final {
				Widget::render(renderers, x, y, width, height);
				box->render(renderers, x, y, width, height);
			}

			SizeRequestMode getRequestMode() const final {
				return box->getRequestMode();
			}

			void measure(const RendererContext &renderers, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) final {
				return box->measure(renderers, orientation, for_width, for_height, minimum, natural);
			}

		private:
			AgentPtr agent;
			std::shared_ptr<Box> box;
			std::vector<ModulePtr> submodules;
			std::shared_ptr<InventoryModule> firstInventoryModule;
	};
}
