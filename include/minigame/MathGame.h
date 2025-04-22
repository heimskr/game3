#pragma once

#include "data/Identifier.h"
#include "minigame/Minigame.h"
#include "types/UString.h"

#include <chrono>

namespace Game3 {
	class ProgressBar;
	class Texture;
	class TextInput;

	class MathGame: public Minigame {
		public:
			static Identifier ID() { return {"base", "minigame/math"}; }
			static std::string GameName() { return "Math Game"; }
			std::string getGameName() const final { return GameName(); }

			using Minigame::Minigame;

			void init() final;
			void tick(double delta) final;
			void render(const RendererContext &, float x, float y, float width, float height) final;
			void setSize(int width, int height) final;
			void reset() final;
			void onFocus() final;
			void onClose() final;
			void submitScore();

		private:
			struct Equation {
				UString text;
				int64_t answer;
				std::size_t points;
				double duration;
				double secondsLeft;
				std::chrono::system_clock::time_point generationTime;

				Equation(UString text, int64_t answer, std::size_t points, double duration);

				std::size_t getRemainingPoints() const;

				static Equation generate();
			};

			Color equationColor;
			std::optional<Equation> equation;
			std::shared_ptr<TextInput> input;
			std::shared_ptr<ProgressBar> bar;

			void invalidInput();
			void validInput();
	};
}
