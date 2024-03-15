#include "entity/Sheep.h"
#include "game/Game.h"
#include "graphics/BatchSpriteRenderer.h"
#include "graphics/Color.h"
#include "graphics/Recolor.h"
#include "graphics/RendererContext.h"
#include "threading/ThreadContext.h"
#include "util/Util.h"

namespace Game3 {
	Sheep::Sheep():
		Entity(ID()),
		Animal(),
		hue("hue", threadContext.random(0.f, 1.f)),
		saturation("saturation", 0.f, 1.f, sample()),
		valueMultiplier("valueMultiplier", .1f, 1.f, sample()) {}

	void Sheep::render(const RendererContext &renderers) {
		if (texture == nullptr || !isVisible())
			return;

		GamePtr game = getGame();
		SpriteRenderer &sprite_renderer = renderers.batchSprite;
		const auto [offset_x, offset_y, offset_z] = offset.copyBase();

		double texture_x_offset = 0.;
		double texture_y_offset = 0.;
		// Animate if the offset is nonzero.
		if (offset_x != 0. || offset_y != 0.) {
			// Choose an animation frame based on the time.
			texture_x_offset = 8. * ((std::chrono::duration_cast<std::chrono::milliseconds>(getTime() - game->startTime).count() / 200) % 4);
		}

		texture_y_offset = 8. * (int(direction.load()) - 1);

		const auto [row, column] = position.copyBase();

		double fluid_offset = 0.;

		if (auto fluid_tile = getRealm()->tileProvider.copyFluidTile({row, column}); fluid_tile && 0 < fluid_tile->level) {
			fluid_offset = std::sin(game->time * 1.5) + .5;
			renderHeight = 10. + fluid_offset;
		} else {
			renderHeight = 16.;
		}

		const double x = column + offset_x;
		const double y = row    + offset_y - offset_z - fluid_offset / 16.;

		RenderOptions main_options{
			.x = x,
			.y = y,
			.offsetX = texture_x_offset,
			.offsetY = texture_y_offset,
			.sizeX = 16.,
			.sizeY = std::min(16., renderHeight + 8. * offset_z),
		};

		if (!heldLeft && !heldRight) {
			renderBody(renderers, main_options);
			return;
		}

		auto render_held = [&](const Held &held, double x_o, double y_o, bool flip = false, double degrees = 0.) {
			if (held) {
				sprite_renderer(held.texture, {
					.x = x + x_o,
					.y = y + y_o,
					.offsetX = held.offsetX,
					.offsetY = held.offsetY,
					.sizeX = 16.,
					.sizeY = 16.,
					.scaleX = flip? -.5 : .5,
					.scaleY = .5,
					.angle = degrees,
				});
			}
		};

		constexpr static double rotation = 0.;

		switch (direction) {
			case Direction::Up:
				render_held(heldLeft,  -.1, .4, false, -rotation);
				render_held(heldRight, 1.1, .4, true,   rotation);
				break;
			case Direction::Left:
				render_held(heldRight, 0., .5);
				break;
			case Direction::Right:
				render_held(heldLeft, .5, .5);
				break;
			default:
				break;
		}

		renderBody(renderers, main_options);

		switch (direction) {
			case Direction::Down:
				render_held(heldRight, -.1, .5, false, -rotation);
				render_held(heldLeft,  1.1, .5, true,   rotation);
				break;
			case Direction::Left:
				render_held(heldLeft, .5, .5, true);
				break;
			case Direction::Right:
				render_held(heldRight, 1., .5, true);
				break;
			default:
				break;
		}
	}

	void Sheep::renderBody(const RendererContext &renderers, const RenderOptions &options) {
		if (!mask)
			mask = getGame()->registry<TextureRegistry>().at("base:texture/sheep_mask");
		renderers.recolor.drawOnMap(texture, mask, options, hue.getValue(), saturation.getValue(), valueMultiplier.getValue());
	}

	bool Sheep::canAbsorbGenes(const nlohmann::json &genes) const {
		return checkGenes(genes, {"hue", "saturation", "valueMultiplier"});
	}

	void Sheep::absorbGenes(const nlohmann::json &genes) {
		absorbGene(hue, genes, "hue");
		absorbGene(saturation, genes, "saturation");
		absorbGene(valueMultiplier, genes, "valueMultiplier");
	}

	void Sheep::iterateGenes(const std::function<void(Gene &)> &function) {
		function(hue);
		function(saturation);
		function(valueMultiplier);
	}

	void Sheep::encode(Buffer &buffer) {
		Animal::encode(buffer);
		buffer << hue;
		buffer << saturation;
		buffer << valueMultiplier;
	}

	void Sheep::decode(Buffer &buffer) {
		Animal::decode(buffer);
		buffer >> hue;
		buffer >> saturation;
		buffer >> valueMultiplier;
	}

	float Sheep::sample() {
		return std::min(1.f, std::max(0.f, std::normal_distribution<float>{.75f, .75f / 3.f}(threadContext.rng)));
	}
}
