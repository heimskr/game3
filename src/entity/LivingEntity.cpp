#include "entity/LivingEntity.h"
#include "entity/TextParticle.h"
#include "game/ServerGame.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
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

	void LivingEntity::renderUpper(const RendererContext &renderers) {
		Entity::renderUpper(renderers);

		if (!canShowHealthBar())
			return;

		RectangleRenderer &rectangle = renderers.rectangle;

		constexpr static double bar_offset = .15;
		constexpr static double bar_width  = .8;
		constexpr static double bar_height = .18;
		constexpr static double thickness  = .05;

		const auto [row, column] = getPosition();
		const auto [x, y, z] = offset.copyBase();

		const double bar_x = double(column) + x - (bar_width - 1) / 2;
		const double bar_y = double(row) + y - z - bar_offset - bar_height;
		const double fraction = double(health) / getMaxHealth();

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

	bool LivingEntity::setHealth(HitPoints new_health) {
		new_health = std::min(getMaxHealth(), new_health);
		const bool changed = new_health != health;
		health = new_health;

		if (getSide() == Side::Server) {
			if (health <= 0)
				kill();
			else
				game->toServer().broadcast(LivingEntityHealthChangedPacket(*this));
		}

		return changed;
	}

	bool LivingEntity::heal(HitPoints to_heal) {
		return setHealth(health + to_heal);
	}

	bool LivingEntity::takeDamage(HitPoints damage) {
		assert(getSide() == Side::Server);

		std::uniform_int_distribution<int> defense_distribution(0, 99);

		const int defense = getDefense();
		const double luck = getLuck();

		for (int roll = 0; roll < defense; ++roll) {
			if (damage == 0)
				break;

			if (defense_distribution(threadContext.rng) < 10 * luck)
				--damage;
		}

		Color color{1, 0, 0, 1};
		if (damage == 0)
			color = {0, 0, 1, 1};

		getRealm()->spawn<TextParticle>(getPosition(), std::to_string(damage), color, .666f);

		if (damage == 0)
			return false;

		if (health.fetch_sub(damage) <= damage) {
			health = 0;
			kill();
			return true;
		}

		game->toServer().broadcast(LivingEntityHealthChangedPacket(*this));
		return false;
	}

	void LivingEntity::kill() {
		assert(getSide() == Side::Server);

		RealmPtr realm = getRealm();

		for (const ItemStack &stack: getDrops())
			stack.spawn(realm, getPosition());

		queueDestruction();
	}

	void LivingEntity::onAttack(const std::shared_ptr<LivingEntity> &) {}

	std::vector<ItemStack> LivingEntity::getDrops() {
		return {};
	}

	double LivingEntity::getBaseSpeed() {
		return 1.5;
	}
}
