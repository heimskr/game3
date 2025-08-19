#include "game/Agent.h"
#include "game/ClientGame.h"
#include "fluid/Fluid.h"
#include "game/FluidContainer.h"
#include "game/HasFluids.h"
#include "ui/module/FluidsModule.h"
#include "ui/widget/Box.h"
#include "ui/widget/Grid.h"
#include "ui/widget/Label.h"
#include "ui/widget/ProgressBar.h"
#include "ui/UIContext.h"

#include <cassert>

namespace Game3 {
	FluidsModule::FluidsModule(UIContext &ui, float selfScale, const std::shared_ptr<ClientGame> &, const std::any &argument, bool show_header):
		FluidsModule(ui, selfScale, std::any_cast<AgentPtr>(argument), show_header) {}

	FluidsModule::FluidsModule(UIContext &ui, float selfScale, const AgentPtr &agent, bool show_header):
		FluidsModule(ui, selfScale, std::dynamic_pointer_cast<HasFluids>(agent), show_header) {}

	FluidsModule::FluidsModule(UIContext &ui, float selfScale, std::shared_ptr<HasFluids> fluid_haver, bool show_header):
		Module(ui, selfScale), fluidHaver(std::move(fluid_haver)), showHeader(show_header) {}

	void FluidsModule::init() {
		auto vbox = std::make_shared<Box>(ui, selfScale);
		vbox->insertAtEnd(shared_from_this());

		if (showHeader) {
			auto label = std::make_shared<Label>(ui, selfScale);
			if (auto agent = std::dynamic_pointer_cast<Agent>(fluidHaver)) {
				label->setText(agent->getName());
			} else {
				label->setText("???");
			}
			label->insertAtEnd(vbox);
		}

		vbox->setHorizontalExpand(true);

		grid = std::make_shared<Grid>(ui, selfScale);
		grid->setHorizontalExpand(true);
		grid->insertAtEnd(vbox);

		updateFluids();
	}

	void FluidsModule::render(const RendererContext &renderers, float x, float y, float width, float height) {
		Module::render(renderers, x, y, width, height);

		updateFluids();

		firstChild->render(renderers, x, y, width, height);
	}

	SizeRequestMode FluidsModule::getRequestMode() const {
		return firstChild->getRequestMode();
	}

	void FluidsModule::measure(const RendererContext &renderers, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) {
		assert(firstChild != nullptr);
		firstChild->measure(renderers, orientation, for_width, for_height, minimum, natural);
	}

	std::pair<std::shared_ptr<Label>, std::shared_ptr<ProgressBar>> FluidsModule::makePair(Color bar_interior) const {
		auto bar = std::make_shared<ProgressBar>(ui, selfScale, bar_interior);
		bar->setFixedHeight(12 * selfScale);
		bar->setHorizontalExpand(true);

		auto label = std::make_shared<Label>(ui, selfScale);
		label->setVerticalAlignment(Alignment::Center);

		return std::make_pair(std::move(label), std::move(bar));
	}

	void FluidsModule::updateFluids() {
		std::shared_ptr<FluidContainer> container = fluidHaver->fluidContainer;
		Lockable<FluidContainer::Map> &levels = container->levels;
		ClientGamePtr game = ui.getGame();
		auto lock = levels.sharedLock();

		for (auto iter = widgetPairs.begin(), end = widgetPairs.end(); iter != end;) {
			const FluidID fluid_id = iter->first;
			if (!levels.contains(fluid_id)) {
				iteratorsByID.erase(fluid_id);
				iter = widgetPairs.erase(iter);
			} else {
				++iter;
			}
		}

		bool should_regrid = false;

		for (const auto &[fluid_id, amount]: levels) {
			const auto max = fluidHaver->getMaxLevel(fluid_id);
			const float progress = static_cast<float>(amount) / static_cast<float>(max);

			if (auto iter_iter = iteratorsByID.find(fluid_id); iter_iter != iteratorsByID.end()) {
				const auto &[label, bar] = iter_iter->second->second;
				bar->setProgress(progress);
			} else {
				std::shared_ptr<Fluid> fluid = game->getFluid(fluid_id);
				auto [iter, inserted] = widgetPairs.emplace(fluid_id, makePair(fluid->color));
				assert(inserted);
				const auto &[label, bar] = iter->second;
				label->setText(fluid->name);
				bar->setProgress(progress);
				should_regrid = true;
				iteratorsByID[fluid_id] = iter;
			}
		}

		lock.unlock();

		if (!should_regrid) {
			return;
		}

		grid->clearChildren();
		std::size_t row = 0;

		for (const auto &[name, pair]: widgetPairs) {
			grid->attach(pair.first, row, 0);
			grid->attach(pair.second, row, 1);
			++row;
		}
	}
}
