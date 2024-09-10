#pragma once

#include "ui/gl/module/Module.h"
#include "ui/gl/widget/Box.h"

#include <any>
#include <memory>

namespace Game3 {
	class Box;
	class Gene;
	class Mutator;

	class GeneInfoModule: public Module {
		public:
			static Identifier ID() { return {"base", "module/gene_info"}; }

			GeneInfoModule(std::shared_ptr<Gene>);

			Identifier getID() const final { return ID(); }
			void init(UIContext &) final;

			using Module::render;
			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;

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