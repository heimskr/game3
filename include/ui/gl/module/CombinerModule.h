#pragma once

#include "types/Types.h"
#include "ui/gl/module/Module.h"
#include "ui/gl/module/MultiModule.h"

namespace Game3 {
	class Box;
	class Combiner;
	class TextInput;

	class CombinerModule: public Module {
		public:
			static Identifier ID() { return {"base", "module/combiner"}; }

			CombinerModule(UIContext &, const ClientGamePtr &, const std::any &);
			CombinerModule(UIContext &, const ClientGamePtr &, std::shared_ptr<Combiner>);

			Identifier getID() const final { return ID(); }
			void init() final;

			using Module::render;
			void render(const RendererContext &, float x, float y, float width, float height) final;
			SizeRequestMode getRequestMode() const final;
			void measure(const RendererContext &, Orientation, float for_width, float for_height, float &minimum, float &natural) final;

			std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final;
			void setInventory(std::shared_ptr<ClientInventory>) final;
			std::shared_ptr<InventoryModule> getPrimaryInventoryModule() final;

		private:
			std::shared_ptr<Combiner> combiner;
			std::shared_ptr<Box> vbox;
			std::shared_ptr<TextInput> textInput;
			std::shared_ptr<MultiModule<Substance::Item, Substance::Energy>> multiModule;

			void setTarget(const std::string &);
	};
}
