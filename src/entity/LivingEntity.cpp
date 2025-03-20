#include "biology/Gene.h"
#include "entity/LivingEntity.h"
#include "entity/SquareParticle.h"
#include "entity/TextParticle.h"
#include "game/ServerGame.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/RenderOptions.h"
#include "lib/JSON.h"
#include "packet/LivingEntityHealthChangedPacket.h"
#include "statuseffect/StatusEffect.h"
#include "threading/ThreadContext.h"

namespace Game3 {
	LivingEntity::LivingEntity():
		Entity("base:invalid/LivingEntity") {}

	void LivingEntity::onSpawn() {
		Entity::onSpawn();
		health = getMaxHealth();
	}

	void LivingEntity::tick(const TickArgs &args) {
		Entity::tick(args);

		auto self = std::dynamic_pointer_cast<LivingEntity>(shared_from_this());

		if (getSide() == Side::Server) {
			if (std::optional<FluidTile> fluid_tile = getRealm()->tileProvider.copyFluidTile(getPosition()); fluid_tile && 0 < fluid_tile->level) {
				FluidPtr fluid = getGame()->getFluid(fluid_tile->id);
				assert(fluid != nullptr);
				fluid->onCollision(self);
			}
		}

		if (statusEffects.empty()) {
			return;
		}

		std::erase_if(statusEffects, [&](const auto &item) {
			return item.second->apply(self, args.delta);
		});
	}

	void LivingEntity::toJSON(boost::json::value &json) const {
		auto &object = json.emplace_object();
		auto this_lock = sharedLock();
		object["health"] = health;
		object["luck"] = luckStat;
	}

	void LivingEntity::absorbJSON(const GamePtr &, const boost::json::value &json) {
		if (json.is_null()) {
			return;
		}

		const auto &object = json.as_object();

		auto this_lock = uniqueLock();

		if (auto *value = object.if_contains("health")) {
			health = boost::json::value_to<HitPoints>(*value);
		}

		if (auto *value = object.if_contains("luck")) {
			luckStat = getDouble(*value);
		}
	}

	void LivingEntity::renderUpper(const RendererContext &renderers) {
		Entity::renderUpper(renderers);

		if (!canShowHealthBar()) {
			return;
		}

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

	bool LivingEntity::isAffectedByKnockback() const {
		return true;
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

	bool LivingEntity::atFullHealth() const {
		return getHealth() == getMaxHealth();
	}

	bool LivingEntity::setHealth(HitPoints new_health) {
		new_health = std::min(getMaxHealth(), new_health);
		const bool changed = new_health != health;
		health = new_health;

		if (getSide() == Side::Server) {
			if (health <= 0) {
				kill();
			} else {
				GamePtr game = getGame();
				game->toServer().broadcast(make<LivingEntityHealthChangedPacket>(*this));
			}
		}

		return changed;
	}

	bool LivingEntity::heal(HitPoints to_heal) {
		return setHealth(health + to_heal);
	}

	bool LivingEntity::takeDamage(HitPoints damage) {
		assert(getSide() == Side::Server);

		if (isInvincible()) {
			return false;
		}

		std::uniform_int_distribution<int> defense_distribution(0, 99);

		const int defense = getDefense();
		const double luck = getLuck();

		for (int roll = 0; roll < defense; ++roll) {
			if (damage == 0) {
				break;
			}

			if (defense_distribution(threadContext.rng) < 10 * luck) {
				--damage;
			}
		}

		Color color{1, 0, 0, 1};
		if (damage == 0) {
			color = {0, 0, 1, 1};
		}

		RealmPtr realm = getRealm();
		realm->spawn<TextParticle>(getPosition(), std::to_string(damage), color, .666f);

		if (damage == 0) {
			return false;
		}

		spawnBlood(3);

		if (health.fetch_sub(damage) <= damage) {
			health = 0;
			kill();
			return true;
		}

		GamePtr game = realm->getGame();
		game->toServer().broadcast(make<LivingEntityHealthChangedPacket>(*this));
		return false;
	}

	void LivingEntity::kill() {
		assert(getSide() == Side::Server);

		RealmPtr realm = getRealm();

		for (const ItemStackPtr &stack: getDrops()) {
			stack->spawn(Place{getPosition(), realm});
		}

		queueDestruction();
	}

	void LivingEntity::spawnBlood(size_t count) {
		RealmPtr realm = getRealm();
		Position position = getPosition();

		std::uniform_real_distribution y_distribution(-0.15, 0.15);
		std::uniform_real_distribution z_distribution(6., 10.);
		std::uniform_real_distribution depth_distribution(-0.5, 0.0);
		std::uniform_real_distribution red_distribution(0.333, 1.0);

		for (double x: {-1, +1}) {
			std::uniform_real_distribution x_distribution(x - 1.0, x + 1.0);
			for (size_t i = 0; i < count; ++i) {
				Vector3 velocity{
					x_distribution(threadContext.rng),
					y_distribution(threadContext.rng),
					z_distribution(threadContext.rng),
				};
				float red = red_distribution(threadContext.rng);
				double depth = depth_distribution(threadContext.rng);
				realm->spawn<SquareParticle>(position, velocity, 0.2, Color{red, 0, 0, 1}, depth, 4)->offset.withUnique([](Vector3 &offset) {
					offset.y += 0.5;
				});
			}
		}
	}

	void LivingEntity::onAttack(const std::shared_ptr<LivingEntity> &) {}

	std::vector<ItemStackPtr> LivingEntity::getDrops() {
		return {};
	}

	bool LivingEntity::canAbsorbGenes(const boost::json::value &) const {
		return false;
	}

	void LivingEntity::absorbGenes(const boost::json::value &) {}

	float LivingEntity::getBaseSpeed() {
		return 1.5;
	}

	void LivingEntity::iterateGenes(const std::function<void(Gene &)> &) {}

	void LivingEntity::iterateGenes(const std::function<void(const Gene &)> &) const {}

	void LivingEntity::inflictStatusEffect(std::unique_ptr<StatusEffect> &&effect, bool can_overwrite) {
		auto iter = statusEffects.find(effect->identifier);
		if (iter == statusEffects.end()) {
			Identifier identifier = effect->identifier;
			statusEffects.emplace(std::move(identifier), std::move(effect));
		} else if (can_overwrite) {
			iter->second = std::move(effect);
		}
	}

	bool LivingEntity::checkGenes(const boost::json::value &genes, std::unordered_set<std::string> &&names) {
		const auto &object = genes.as_object();

		if (object.size() != names.size()) {
			return false;
		}

		for (const auto &[key, value]: object) {
			names.erase(key);
		}

		return names.size() == 0;
	}
}
