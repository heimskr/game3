#include "entity/LivingEntity.h"
#include "game/ServerGame.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererSet.h"
#include "graphics/RenderOptions.h"
#include "packet/LivingEntityHealthChangedPacket.h"
#include "threading/ThreadContext.h"

namespace Game3 {
	LivingEntity::LivingEntity():
		Entity("base:invalid/LivingEntity") {}

	void LivingEntity::onSpawn() {
		Entity::onSpawn();
		health = getMaxHealth();
	}

	void LivingEntity::toJSON(nlohmann::json &json) const {
		auto this_lock = sharedLock();
		json["health"] = health;
		json["luck"] = luckStat;
	}

	void LivingEntity::absorbJSON(Game &, const nlohmann::json &json) {
		if (json.is_null())
			return;

		auto this_lock = uniqueLock();

		if (auto iter = json.find("health"); iter != json.end())
			health = *iter;

		if (auto iter = json.find("luck"); iter != json.end())
			luckStat = *iter;
	}

	void LivingEntity::render(const RendererSet &renderers) {
		Entity::render(renderers);

		if (!canShowHealthBar())
			return;

		RectangleRenderer &rectangle = renderers.rectangle;

		constexpr static float bar_offset = .15f;
		constexpr static float bar_width  = .8f;
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
		buffer << defenseStat;
		buffer << luckStat;
	}

	void LivingEntity::decode(Buffer &buffer) {
		auto this_lock = uniqueLock();
		buffer >> health;
		buffer >> defenseStat;
		buffer >> luckStat;
	}

	bool LivingEntity::canShowHealthBar() const {
		const auto max = getMaxHealth();
		return !isInvincible() && max != 0 && health != max;
	}

	int LivingEntity::getDefense() const {
		return defenseStat;
	}

	double LivingEntity::getLuck() const {
		return luckStat;
	}

	void LivingEntity::setHealth(HitPoints new_health) {
		health = new_health;
	}

	void LivingEntity::takeDamage(HitPoints damage) {
		assert(getSide() == Side::Server);

		std::uniform_int_distribution<int> defense_distribution(0, 99);

		const int defense = getDefense();
		const double luck = getLuck();

		for (int i = 0; i < defense; ++i) {
			if (damage == 0)
				break;

			if (defense_distribution(threadContext.rng) < 10 * luck)
				--damage;
		}

		if (damage == 0)
			return;

		if (health.fetch_sub(damage) <= damage) {
			health = 0;
			kill();
		} else {
			game->toServer().broadcast(LivingEntityHealthChangedPacket(*this));
		}
	}

	void LivingEntity::kill() {
		assert(getSide() == Side::Server);
		queueDestruction();
	}
}
