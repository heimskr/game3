#include "entity/LivingEntity.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererSet.h"
#include "graphics/RenderOptions.h"

namespace Game3 {
	LivingEntity::LivingEntity():
		Entity("base:invalid/LivingEntity") {}

	void LivingEntity::onCreate() {
		health = getMaxHealth();
	}

	void LivingEntity::toJSON(nlohmann::json &json) const {
		auto this_lock = sharedLock();
		json["health"] = health;
	}

	void LivingEntity::absorbJSON(Game &, const nlohmann::json &json) {
		if (json.is_null())
			return;

		auto this_lock = uniqueLock();

		if (auto iter = json.find("health"); iter != json.end())
			health = *iter;
	}

	void LivingEntity::render(const RendererSet &renderers) {
		Entity::render(renderers);

		if (!canShowHealthBar())
			return;

		RectangleRenderer &rectangle = renderers.rectangle;

		constexpr static float bar_offset = .15f;
		constexpr static float bar_width  = 1.5f;
		constexpr static float bar_height = .18f;
		constexpr static float thickness  = .05f;

		const auto [row, column] = getPosition();
		const auto [x, y, z] = offset.copyBase();

		const float bar_x = float(column) + x - (bar_width - 1) / 2;
		const float bar_y = float(row) + y - z - bar_offset - bar_height;
		const float fraction = double(health) / getMaxHealth();

		rectangle.drawOnMap(RenderOptions {
			.x = bar_x - thickness,
			.y = bar_y - thickness,
			.sizeX = bar_width  + thickness * 2,
			.sizeY = bar_height + thickness * 2,
			.color = {.1, .1, .1, .9},
		});

		rectangle.drawOnMap(RenderOptions {
			.x = bar_x,
			.y = bar_y,
			.sizeX = bar_width * fraction,
			.sizeY = bar_height,
			.color = {0, 1, 0, 1},
		});

		rectangle.drawOnMap(RenderOptions {
			.x = bar_x + fraction * bar_width,
			.y = bar_y,
			.sizeX = bar_width * (1 - fraction),
			.sizeY = bar_height,
			.color = {1, 0, 0, 1},
		});
	}

	void LivingEntity::encode(Buffer &buffer) {
		auto this_lock = sharedLock();
		buffer << health;
	}

	void LivingEntity::decode(Buffer &buffer) {
		auto this_lock = uniqueLock();
		buffer >> health;
	}

	bool LivingEntity::canShowHealthBar() const {
		const auto max = getMaxHealth();
		return !isInvincible() && max != 0 && health != max;
	}
}
