#include "entity/Projectile.h"
#include "game/ClientGame.h"
#include "graphics/BatchSpriteRenderer.h"
#include "graphics/ItemTexture.h"
#include "graphics/RendererContext.h"
#include "graphics/SpriteRenderer.h"
#include "registry/Registries.h"
#include "ui/Window.h"

namespace {
	constexpr double GRAVITY = 32;
}

namespace Game3 {
	Projectile::Projectile(EntityType type, Identifier item_id, const Vector3 &initial_velocity, double angular_velocity, double linger_time):
		Entity(std::move(type)), itemID(std::move(item_id)), initialVelocity(initial_velocity), angularVelocity(angular_velocity), lingerTime(linger_time) {}

	void Projectile::tick(const TickArgs &args) {
		auto offset_lock = offset.uniqueLock();
		auto velocity_lock = velocity.uniqueLock();

		velocity.z -= GRAVITY * args.delta;
		offset.z = std::max(offset.z + args.delta * velocity.z, 0.);
		offset.x += args.delta * velocity.x;
		offset.y += args.delta * velocity.y;

		if (offset.z <= 0.) {
			velocity.getBase() = {};
			age += args.delta;
			if (lingerTime <= age) {
				onExpire();
				return;
			}
		} else {
			angle += angularVelocity * args.delta;

			if (!hasHit) {
				Position position = getPosition();
				Vector2d self_vector(position);
				position.row += offset.y;
				position.column += offset.x;
				self_vector.x += offset.x;
				self_vector.y += offset.y;
				auto self = getSelf();

				for (const EntityPtr &target: getRealm()->findEntitiesSquare(position, 2)) {
					if (target != self && target->getGID() != thrower && target->isAffectedByKnockback()) {
						Vector2d target_vector(target->getPosition());
						target_vector.x += target->offset.x;
						target_vector.y += target->offset.y;
						if (self_vector.distance(target_vector) < 1) { // TODO: get actual size and use that for the threshold
							onHit(target);
							break;
						}
					}
				}
			}
		}

		enqueueTick();
	}

	void Projectile::render(const RendererContext &renderers) {
		SpriteRenderer &sprite_renderer = renderers.batchSprite;

		if (!isVisible()) {
			return;
		}

		if (texture == nullptr || needsTexture) {
			if (needsTexture) {
				setTexture(sprite_renderer.window->game);
				needsTexture = false;
			} else {
				return;
			}
		}

		const float x = position.column + offset.x;
		const float y = position.row + offset.y - offset.z;

		sprite_renderer(texture, {
			.x = x + .125f / scale,
			.y = y + .125f / scale,
			.offsetX = offsetX,
			.offsetY = offsetY,
			.sizeX = sizeX,
			.sizeY = sizeY,
			.scaleX = scale * 16.f / sizeX,
			.scaleY = scale * 16.f / sizeY,
			.angle = angle,
		});
	}

	void Projectile::onSpawn() {
		Entity::onSpawn();
		velocity = initialVelocity;
	}

	bool Projectile::shouldBroadcastDestruction() const {
		// Clients will handle projectile destruction themselves.
		return false;
	}

	int Projectile::getZIndex() const {
		return 2;
	}

	void Projectile::encode(Buffer &buffer) {
		Entity::encode(buffer);
		buffer << lingerTime;
		buffer << angularVelocity;
		buffer << angle;
		buffer << hasHit;
		buffer << thrower;
	}

	void Projectile::decode(Buffer &buffer) {
		Entity::decode(buffer);
		buffer >> lingerTime;
		buffer >> angularVelocity;
		buffer >> angle;
		buffer >> hasHit;
		buffer >> thrower;
	}

	const Identifier & Projectile::getItemID() const {
		return itemID;
	}

	void Projectile::onHit(const EntityPtr &) {
		hasHit = true;
		velocity.x = 0;
		velocity.y = 0;
	}

	void Projectile::onExpire() {
		queueDestruction();
	}

	void Projectile::setThrower(const EntityPtr &entity) {
		thrower = entity->getGID();
	}

	std::shared_ptr<Texture> Projectile::getTexture() {
		GamePtr game = getGame();
		return ItemStack::create(game, getItemID())->getItemTexture(*game)->getTexture();
	}

	void Projectile::setTexture(const ClientGamePtr &game) {
		std::shared_ptr<ItemTexture> item_texture = game->registry<ItemTextureRegistry>().at(getItemID());
		texture = getTexture();
		texture->init();
		offsetX = item_texture->x / 2.f;
		offsetY = item_texture->y / 2.f;
		sizeX = float(item_texture->width);
		sizeY = float(item_texture->height);
	}

	void Projectile::applyKnockback(const EntityPtr &target, float factor) {
		assert(target.get() != this);

		target->velocity.withUnique([this, factor](Vector3 &target_velocity) {
			auto lock = velocity.uniqueLock();
			target_velocity.x += velocity.x;
			target_velocity.y += velocity.y;
			target_velocity.z = std::max(target_velocity.z, std::abs(velocity.z) * factor);
			velocity.x = 0;
			velocity.y = 0;
			velocity.z = 0;
		});

		target->offset.withUnique([](Vector3 &offset) {
			offset.z += 1;
		});

		target->increaseUpdateCounter();
		target->sendToVisible();

		increaseUpdateCounter();
		sendToVisible();
	}
}
