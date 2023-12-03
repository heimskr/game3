#include "algorithm/DamageCalculation.h"
#include "entity/Monster.h"
#include "game/ServerGame.h"
#include "realm/Realm.h"

#include <cassert>

namespace Game3 {
	namespace {
		constexpr float PATIENCE = 10.f;
		constexpr HitPoints MAX_HEALTH = 30;
	}

	Monster::Monster():
		Entity("base:invalid/Monster") {}

	void Monster::tick(Game &game, float delta) {
		Entity::tick(game, delta);

		if (!isAttacking())
			return;

		if (!tryAttack()) {
			timeSinceAttack += delta;
			if (getPatience() < timeSinceAttack)
				giveUp();
		}
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
		weakTarget = attacker;
		targetGID = attacker->getGID();
		timeSinceAttack = 0.f;
	}

	float Monster::getPatience() const {
		return PATIENCE;
	}

	LivingEntityPtr Monster::getTarget() {
		if (auto locked = weakTarget.lock())
			return locked;

		giveUp();
		return nullptr;
	}

	bool Monster::isAttacking() {
		return getTarget() != nullptr;
	}

	void Monster::giveUp() {
		weakTarget.reset();
		targetGID = -1;
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

		assert(getSide() == Side::Server);
		getGame().toServer().entityTeleported(*this, MovementContext{
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

		const Direction facing = position.getFacing(target->getPosition());

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

		WARN("Couldn't find any position to move to.");
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
		timeSinceAttack = 0.f;
	}
}
