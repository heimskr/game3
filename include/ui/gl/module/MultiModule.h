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
#include "ui/gl/widget/Scroller.h"
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
				return {"base", "module/multi_" + getSuffix()};
			}

			static std::string getSuffix() {
				const static std::map<Substance, char> substances{
					{Substance::Item,   'i'},
					{Substance::Fluid,  'f'},
					{Substance::Energy, 'e'},
				};

				std::string suffix;
				for (Substance substance: {S...})
					suffix += substances.at(substance);
				return suffix;
			}

			MultiModule(ClientGamePtr game, const std::any &argument):
				MultiModule(std::move(game), std::any_cast<AgentPtr>(argument)) {}

			MultiModule(ClientGamePtr game, const AgentPtr &agent):
				weakGame(game), agent(std::move(agent)) {}

			Identifier getID() const final {
				return ID();
			}

			void init(UIContext &ui) final {
				ClientGamePtr game = weakGame.lock();
				assert(game);

				for (Substance substance: {S...}) {
					switch (substance) {
						case Substance::Item: {
							auto inventoried = std::dynamic_pointer_cast<InventoriedTileEntity>(agent);
							assert(inventoried);
							for (size_t i = 0; i < inventoried->getInventoryCount(); ++i) {
								auto inventory = safeDynamicCast<ClientInventory>(inventoried->getInventory(i));
								submodules.emplace_back(std::make_shared<InventoryModule>(game, std::move(inventory)));
							}
							break;
						}

						case Substance::Fluid: {
							assert(std::dynamic_pointer_cast<FluidHoldingTileEntity>(agent));
							submodules.emplace_back(std::make_shared<FluidsModule>(game, agent));
							break;
						}

						case Substance::Energy: {
							submodules.emplace_back(std::make_shared<EnergyModule>(game, agent, false));
							break;
						}

						default:
							throw std::invalid_argument(std::format("Invalid Substance: {}", static_cast<int>(substance)));
					}
				}

				box = std::make_shared<Box>(scale, Orientation::Vertical);
				box->insertAtEnd(shared_from_this());

				for (const ModulePtr &submodule: submodules) {
					submodule->insertAtEnd(box);
					submodule->init(ui);
				}
			}

			void reset() final {
				for (const ModulePtr &submodule: submodules)
					submodule->reset();
			}

			void update() final {
				for (const ModulePtr &submodule: submodules)
					submodule->update();
			}

			std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final {
				for (const ModulePtr &submodule: submodules)
					if (std::optional<Buffer> buffer = submodule->handleMessage(source, name, data))
						return buffer;
				return std::nullopt;
			}

			void render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) final {
				Widget::render(ui, renderers, x, y, width, height);
				box->render(ui, renderers, x, y, width, height);
			}

			SizeRequestMode getRequestMode() const final {
				return box->getRequestMode();
			}

			void measure(const RendererContext &renderers, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) final {
				return box->measure(renderers, orientation, for_width, for_height, minimum, natural);
			}

		private:
			std::weak_ptr<ClientGame> weakGame;
			AgentPtr agent;
			std::shared_ptr<Box> box;
			std::vector<ModulePtr> submodules;
	};
}
