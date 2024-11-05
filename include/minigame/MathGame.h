#pragma once

#include "data/Identifier.h"
#include "minigame/Minigame.h"
#include "types/UString.h"

namespace Game3 {
	class Texture;
	class TextInput;

	class MathGame: public Minigame {
		public:
			static Identifier ID() { return {"base", "minigame/math"}; }
			std::string getGameName() const final { return "Math Game"; }

			using Minigame::Minigame;

			void init() final;
			void tick(double delta) final;
			void render(const RendererContext &, float x, float y, float width, float height) final;
			void setSize(int width, int height) final;
			void reset() final;

		private:
			struct Equation {
				UString text;
				int64_t answer;
				std::size_t score;

				static Equation generate();
			};

			std::optional<Equation> equation;
			std::shared_ptr<TextInput> input;
	};
}
