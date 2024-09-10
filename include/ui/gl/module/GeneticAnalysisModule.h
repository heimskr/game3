#pragma once

#include "types/Types.h"
#include "ui/gl/module/Module.h"

#include <any>
#include <memory>
#include <vector>

namespace Game3 {
	class Box;

	class GeneticAnalysisModule: public Module {
		public:
			static Identifier ID() { return {"base", "module/genetic_analysis"}; }

			GeneticAnalysisModule(ClientGamePtr, const std::any &);
			GeneticAnalysisModule();

			Identifier getID() const final { return ID(); }
			void init(UIContext &) final;
			void reset() final;
			void update() final;
			void update(const ItemStackPtr &);

			void render(UIContext &ui, const RendererContext &renderers, float x, float y, float width, float height) final;
			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &renderers, Orientation orientation, float for_width, float for_height, float &minimum, float &natural) final;

		private:
			std::shared_ptr<Box> vbox;

			void populate();
			void ponderOrb(const ItemStackPtr &);
			void analyzeGene(const ItemStackPtr &);
			void analyzeTemplate(const ItemStackPtr &);
			void addLabel(const std::string &);
			void clearText();
	};
}
