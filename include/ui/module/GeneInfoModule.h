#pragma once

#include "ui/module/Module.h"
#include "ui/widget/Box.h"

#include <any>
#include <memory>

namespace Game3 {
	class Box;
	class Gene;
	class Mutator;

	class GeneInfoModule: public Module {
		public:
			static Identifier ID() { return {"base", "module/gene_info"}; }

			GeneInfoModule(UIContext &, float selfScale, std::shared_ptr<Gene>);

			Identifier getID() const final { return ID(); }
			void init() final;

			using Module::render;
			void render(const RendererContext &, float x, float y, float width, float height) final;

			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

			void reset() final;
			void update() final;
			void update(std::shared_ptr<Gene>);

		private:
			std::shared_ptr<Gene> gene;
			std::shared_ptr<Box> vbox;
	};
}