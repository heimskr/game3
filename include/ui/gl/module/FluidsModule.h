#pragma once

#include "types/Types.h"
#include "ui/gl/module/Module.h"

#include <any>
#include <map>
#include <memory>

namespace Game3 {
	class ClientGame;
	class ClientInventory;
	class Grid;
	class HasFluids;
	class Label;
	class ProgressBar;
	struct Fluid;

	class FluidsModule: public Module {
		public:
			FluidsModule(std::shared_ptr<ClientGame>, const std::any &, bool show_header = true);
			FluidsModule(std::shared_ptr<ClientGame>, const AgentPtr &, bool show_header = true);
			FluidsModule(std::shared_ptr<HasFluids>, bool show_header = true);

			static Identifier ID() { return {"base", "module/fluid_levels"}; }

			Identifier getID() const final { return ID(); }
			void init(UIContext &) final;

			using Module::render;
			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

		private:
			std::shared_ptr<HasFluids> fluidHaver;
			std::shared_ptr<Grid> grid;
			std::map<FluidID, std::pair<std::shared_ptr<Label>, std::shared_ptr<ProgressBar>>> widgetPairs;
			std::map<FluidID, decltype(widgetPairs)::iterator> iteratorsByID;
			bool showHeader{};

			std::pair<std::shared_ptr<Label>, std::shared_ptr<ProgressBar>> makePair(Color bar_interior) const;
			void updateFluids(UIContext &);
	};
}
