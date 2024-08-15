#include "entity/Projectile.h"
#include "game/ClientGame.h"
#include "graphics/BatchSpriteRenderer.h"
#include "graphics/ItemTexture.h"
#include "graphics/RendererContext.h"
#include "graphics/SpriteRenderer.h"
#include "registry/Registries.h"
#include "ui/Canvas.h"

namespace {
	constexpr double GRAVITY = 32;
}

namespace Game3 {
	Projectile::Projectile(Identifier item_id, const Vector3 &initial_velocity, double linger_time):
		Entity(ID()), itemID(std::move(item_id)), initialVelocity(initial_velocity), lingerTime(linger_time) {}

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
				queueDestruction();
				return;
			}
		}

		enqueueTick();
	}

	void Projectile::render(const RendererContext &renderers) {
		SpriteRenderer &sprite_renderer = renderers.batchSprite;

		if (!isVisible())
			return;

		if (texture == nullptr || needsTexture) {
			if (needsTexture) {
				setTexture(sprite_renderer.canvas->game);
				needsTexture = false;
			} else {
				return;
			}
		}

		const float x = position.column + offset.x;
		const float y = position.row + offset.y - offset.z;

		sprite_renderer(texture, {
			.x = x + .125f,
			.y = y + .125f,
			.offsetX = offsetX,
			.offsetY = offsetY,
			.sizeX = sizeX,
			.sizeY = sizeY,
			.scaleX = .75f * 16.f / sizeX,
			.scaleY = .75f * 16.f / sizeY,
		});
	}

	void Projectile::onSpawn() {
		Entity::onSpawn();
		velocity = initialVelocity;

		if (getSide() == Side::Client) {
		}
	}

	int Projectile::getZIndex() const {
		return 2;
	}

	void Projectile::encode(Buffer &buffer) {
		Entity::encode(buffer);
		buffer << lingerTime;
	}

	void Projectile::decode(Buffer &buffer) {
		Entity::decode(buffer);
		buffer >> lingerTime;
	}

	const Identifier & Projectile::getItemID() const {
		return itemID;
	}

	std::shared_ptr<Texture> Projectile::getTexture() {
		GamePtr game = getGame();
		return ItemStack::create(game, getItemID())->getTexture(*game);
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
}
