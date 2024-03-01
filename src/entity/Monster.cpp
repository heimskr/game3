#include "algorithm/DamageCalculation.h"
#include "entity/Monster.h"
#include "game/ServerGame.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"
#include "util/Cast.h"
#include "util/Util.h"

#include <cassert>

namespace Game3 {
	namespace {
		constexpr float PATIENCE = 10.f;
		constexpr float SEARCH_PERIOD = 1;
		constexpr float ADJUSTMENT_PERIOD = 0.5;
		constexpr uint64_t SEARCH_RADIUS = 8;
		constexpr uint64_t TENACITY = 16;
		constexpr HitPoints MAX_HEALTH = 30;
	}

	Monster::Monster():
		Entity("base:invalid/Monster") {}

	void Monster::tick(const TickArgs &args) {
		Entity::tick(args);

		const auto delta = args.delta;

		if (getSide() != Side::Server)
			return;

		if (hasTarget()) {
			timeSinceAttack += delta;

			LivingEntityPtr target = getTarget();

			if (std::optional<Position> goal = pathfindGoal.copyBase(); goal && target) {
				const Position target_position = target->getPosition();
				if (*goal != target_position) {
					path = {};
					timeSinceAdjustment += delta;
					if (target_position.taxiDistance(getPosition()) > getTenacity()) {
						giveUp();
					} else if (ADJUSTMENT_PERIOD <= timeSinceAdjustment) {
						timeSinceAdjustment = 0;
						followTarget();
					}
				} else {
					timeSinceAdjustment = 0;
				}
			}

			if (!tryAttack() && getPatience() < timeSinceAttack)
				giveUp();
		} else {
			timeSinceSearch += delta;
			if (SEARCH_PERIOD <= timeSinceSearch)
				search();
		}

		if (canDespawn())
			queueDestruction();
	}

	void Monster::encode(Buffer &buffer) {
		Entity::encode(buffer);
		LivingEntity::encode(buffer);
		buffer << targetGID;
	}

	void Monster::decode(Buffer &buffer) {
		Entity::decode(buffer);
		LivingEntity::decode(buffer);
		buffer >> targetGID;
	}

	HitPoints Monster::getMaxHealth() const {
		return MAX_HEALTH;
	}

	void Monster::onAttack(const std::shared_ptr<LivingEntity> &attacker) {
		setTarget(attacker);
	}

	float Monster::getPatience() const {
		return PATIENCE;
	}

	uint64_t Monster::getSearchRadius() const {
		return SEARCH_RADIUS;
	}

	uint64_t Monster::getTenacity() const {
		return TENACITY;
	}

	bool Monster::canDespawn() const {
		if (targetGID != GlobalID(-1))
			return false;

		RealmPtr realm = getRealm();

		const bool in_range = realm->hasEntitiesSquare(getPosition(), getSearchRadius(), [](const EntityPtr &entity) {
			return entity->isPlayer();
		});

		if (in_range)
			return false;

		return getMinimumAgeForDespawn() < age;
	}

	float Monster::getMinimumAgeForDespawn() const {
		return 30;
	}

	std::vector<ItemStackPtr> Monster::getDrops() {
		std::vector<ItemStackPtr> out = LivingEntity::getDrops();
		std::uniform_int_distribution distribution(0, 9);
		if (distribution(threadContext.rng) < 2)
			out.push_back(ItemStack::create(getGame(), "base:item/morsel"));
		return out;
	}

	LivingEntityPtr Monster::getTarget() {
		if (targetGID == GlobalID(-1))
			return nullptr;

		if (auto locked = weakTarget.lock())
			return locked;

		giveUp();
		return nullptr;
	}

	void Monster::setTarget(const std::shared_ptr<LivingEntity> &new_target) {
		weakTarget = new_target;
		targetGID = new_target->getGID();
	}

	bool Monster::hasTarget() {
		return getTarget() != nullptr;
	}

	void Monster::giveUp() {
		weakTarget.reset();
		targetGID = -1;
		timeSinceSearch = 0;
		timeSinceAdjustment = 0;
	}

	bool Monster::isNearTarget() {
		LivingEntityPtr target = getTarget();

		if (!target)
			return false;

		return target->getPosition().taxiDistance(getPosition()) == 1;
	}

	bool Monster::isFacingTarget() {
		LivingEntityPtr target = getTarget();

		if (!target)
			return false;

		const Position target_position = target->getPosition();

		switch (direction) {
			case Direction::Up:    return position + Position(-1,  0) == target_position;
			case Direction::Right: return position + Position( 0,  1) == target_position;
			case Direction::Down:  return position + Position( 1,  0) == target_position;
			case Direction::Left:  return position + Position( 0, -1) == target_position;
			default:
				return false;
		}
	}

	void Monster::faceTarget() {
		LivingEntityPtr target = getTarget();

		if (!target)
			return;

		const Position target_position = target->getPosition();

		if (position + Position(-1, 0) == target_position) {
			direction = Direction::Up;
		} else if (position + Position(0, 1) == target_position) {
			direction = Direction::Right;
		} else if (position + Position(1, 0) == target_position) {
			direction = Direction::Down;
		} else if (position + Position(0, -1) == target_position) {
			direction = Direction::Left;
		} else {
			return;
		}

		GamePtr game = getGame();
		assert(game->getSide() == Side::Server);
		game->toServer().entityTeleported(*this, MovementContext{
			.facingDirection = direction
		});
	}

	void Monster::followTarget() {
		LivingEntityPtr target = getTarget();

		{
			auto lock = path.sharedLock();
			if (!path.empty() || !target)
				return;
		}

		RealmPtr realm = getRealm();
		Direction facing = target->getPosition().getFacing(position);
		if (facing == Direction::Invalid)
			facing = Direction::Right;

		// First try to pathfind to the closest position that's adjacent to the player.
		Position destination = target->getPosition() + facing;
		if (realm->isPathable(destination)) {
			pathfind(destination, 256);
			return;
		}

		// If that doesn't work, try all the other directions.
		for (const Direction offset: ALL_DIRECTIONS) {
			if (offset == facing)
				continue;

			const Position destination = target->getPosition() + offset;
			if (realm->isPathable(destination)) {
				pathfind(destination, 256);
				return;
			}
		}
	}

	bool Monster::tryAttack() {
		LivingEntityPtr target = getTarget();

		if (!target)
			return false;

		if (!isNearTarget()) {
			followTarget();
			return false;
		}

		if (!isFacingTarget())
			faceTarget();

		attack(target);
		return true;
	}

	void Monster::attack(const std::shared_ptr<LivingEntity> &to_attack) {
		if (to_attack->isInvincible() || timeSinceAttack < getAttackPeriod())
			return;

		const HitPoints damage = calculateDamage(getBaseDamage(), getVariability(), getLuck());
		if (to_attack->takeDamage(damage))
			giveUp();
		timeSinceAttack = 0;
	}

	void Monster::search() {
		timeSinceSearch = 0;
		RealmPtr realm = getRealm();

		std::vector<EntityPtr> in_range = realm->findEntitiesSquare(getPosition(), getSearchRadius(), [](const EntityPtr &entity) {
			return entity->isPlayer();
		});

		if (in_range.empty())
			return;

		setTarget(safeDynamicCast<LivingEntity>(choose(in_range, threadContext.rng)));
	}
}
