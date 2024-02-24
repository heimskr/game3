# Entities

Entities are things like players, NPCs and animals that can move around inside/among realms. Unlike tile entities, multiple entities can occupy the same position.

## Implementing a New Entity

There's a bit of boilerplate involved. First, include `"entity/Entity.h"` and create a class that extends `Game3::Entity` (or just `Entity` if you're in the `Game3` namespace already).

Create a public static method like this:
```c++
static Identifier ID() { return {"base", "entity/my_entity"}; }
```
Replace `my_entity` in that example with the ID of your entity and `base` with your plugin ID if you're adding the entity from a plugin, but keep the `entity/` at the beginning of the second component
of the identifier.

### Serialization

Next, you need to override the serialization methods so that the entity's data can be included in the save file. Add these overridden methods publicly:
```c++
void toJSON(nlohmann::json &json) const override;
void absorbJSON(Game &game, const nlohmann::json &json) override;
```
Begin your implementation of `toJSON` with
```c++
Entity::toJSON(json);
```
and your implementation of `absorbJSON` with
```c++
Entity::absorbJSON(game, json);
```
If you forget to do this, only your own custom data will be included in/extracted from the serialized JSON.

### `create`

If your entity is meant to be spawned (i.e., if it's not just a base class to be extended further, the way `Animal` is), you need a static `create` method. Put the following in your class definition,
replacing `MyEntity` with your entity's class name:
```c++
static std::shared_ptr<MyEntity> create(Game &game) {
	auto out = std::shared_ptr<MyEntity>(new MyEntity());
	out->init(game);
	return out;
}
```

### `fromJSON`

If you add a `create` method, you'll need a static `fromJSON` method too. Add this code, making sure to add `Entity` as a friend class.

```c++
static std::shared_ptr<MyEntity> fromJSON(Game &game, const nlohmann::json &json) {
	auto out = Entity::create<MyEntity>();
	out->absorbJSON(game, json);
	return out;
}

friend class Entity;
```

### Constructors

You'll need at least one constructor, even if your entity is just a base class to be extended further. All constructors should be `protected` so that they can't be called except through the `create` method.
If your entity doesn't have any special properties that need to be specified on construction, you can use something like this, replacing `MyEntity` with your entity's class name:
```c++
MyEntity(): Entity(ID()) {}
```
Feel free to add parameters to the constructor as necessary. Your class is not required to have a default constructor.

### Optional Overrides

There are some virtual methods that are optional to override. These include:
- `void render(const RendererContext &)`: responsible for rendering the entity every frame.
- `void tick(const TickArgs &)`: called every frame unless overridden. The `delta` member of `TickArgs` is the time (in seconds) since the previous frame.
- `void remove()`: called when the entity should be removed. If you want to override this to run some code before the entity is removed, make sure you include `Entity::remove();` *after* your code.
                   If you don't include `Entity::remove();` at all, your entity won't be removed.
- `bool onInteractOn(const PlayerPtr &)`: called when the player is standing on the same square as your entity and presses the same-square interaction key (`E` by default).
                                          Returns true iff anything interesting happened as a result.
- `bool onInteractNextTo(const PlayerPtr &)`: called when the player is standing one square from your entity facing your entity and presses the interaction key (`e` by default).
                                              Returns true iff anything interesting happened as a result.
- `void init(Game &)`: called when the entity is created with the static `create` method.
- `void initAfterLoad(Game &)`: called after the game is loaded from a save file.
- `double getMovementSpeed() const`: returns how many tiles the entity can move per second.
- `std::string getName() const`: returns the display name for the entity. If not overridden, it'll return something like `"Unknown Entity (base:entity/my_entity)"`.

For LivingEntity:
- `HitPoints getMaxHealth() const`: returns the number of hitpoints the entity has when it's at full health.

### Textures

You need to specify the texture for the entity. In a `base:entity_texture_map`-type file (which will be `/data/entity_textures.json` if you're adding to the base game), add an entry like this,
replacing `base` with your plugin namespace if necessary and `my_entity` with your entity's identifier:
```json
"base:entity/my_entity": ["base:texture/my_entity", 3]
```
Replace `3` with the "texture variety" of the entity. A "texture variety" indicates how the sprite sheet for the entity is laid out.
Use `0` if you've overridden the `render` method, `1` if your entity looks like [this](https://github.com/heimskr/game3/blob/master/resources/characters/champions/Gangblanc.png?raw=true)
(five frames of animation), `2` if your entity looks like [this](https://github.com/heimskr/game3/blob/master/resources/characters/blacksmith.png?raw=true) (five frames of animation, with diagonals)
or `3` if your entity looks like [this](https://github.com/heimskr/game3/blob/master/resources/animals/chicken.png?raw=true) (four frames of animation).

Next, you need to specify the details for your texture. (Make sure your texture is square; add transparent pixels if necessary.) In a `base:texture_map`-type file (which will be `/data/textures.json`
if you're adding to the base game), add an entry like this, replacing things as before:
```json
"base:texture/my_entity": ["path/to/spritesheet.png"]
```

### Registering Your Entity

<!-- TODO: after the plugin system is implemented, add instructions for plugins -->
In `Game3::Game::addEntityFactories`, add a line like this:
```c++
add(EntityFactory::create<MyEntity>());
```
This is for base-game entities. Once the plugin system is ready, instructions will be added here on how to register the entity from a plugin.

Once you've done all that, your new entity should be good to go!
