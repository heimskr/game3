# Tile Entities

<!-- TODO: update when Tree is updated so mature trees become regular tiles -->
Tile entities are ways of associating extra data and code with a tile. For example, trees are tile entities because they have age data associated with them and doors are tile entities because they have code to teleport the player on contact. A given tile can hold only one tile entity.

## Implementing a New Tile Entity

First, include `"tileentity/TileEntity.h"` and create a class that extends `Game3::TileEntity` (or just `TileEntity` if you're in the `Game3` namespace already).

Create a public static method like this:
```c++
static Identifier ID() { return {"base", "te/my_tile_entity"}; }
```
Replace `my_tile_entity` in that example with the ID of your tile entity and `base` with your plugin ID if you're adding the tile entity from a plugin, but keep the `te/` at the beginning of the second component of the identifier.

Also, be sure to add `Game3::TileEntity` as a friend class. This is necessary for `TileEntity::create` to work.

### Serialization

Next, you need to override the serialization methods so that the tile entity's data can be included in the save file. Add these overridden methods publicly:
```c++
void toJSON(nlohmann::json &json) const override;
void absorbJSON(Game &game, const nlohmann::json &json) override;
```
Begin your implementation of `toJSON` with
```c++
TileEntity::toJSON(json);
```
and your implementation of `absorbJSON` with
```c++
TileEntity::absorbJSON(game, json);
```
If you forget to do this, only your own custom data will be included in/extracted from the serialized JSON.

### Constructors

All constructors should be `protected` so that they can't be called except through the static `TileEntity::create` method. Your class is required to have a default (parameterless) constructor. This will suffice:
```c++
MyTileEntity() = default;
```

### Optional Overrides

There are some virtual methods that are optional to override. These include:
- `void init(Game &)`: called when the tile entity is created with the static `create` method.
- `void tick(const TickArgs &)`: called once at first. The tile entity is responsible for queueing further ticks with `enqueueTick`. The `delta` member of `TickArgs` is the time (in seconds) since the previous frame.
- `void onSpawn()`: called when the tile entity is spawned new in a realm or added to a realm while the realm is being loaded from JSON.
- `void onRemove()`: called when the tile entity is removed.
- `void onNeighborUpdated(Index row_offset, Index column_offset)`: called when a neighbor is updated. The offsets are in the range [-1, 1]. An "update" is either a tile entity being removed or a tile ID changing.
- `void render(SpriteRenderer &)`: responsible for rendering the tile entity every frame.
- `bool onInteractOn(const PlayerPtr &)`: called when the player is standing on the same square as your tile entity and presses the same-square interaction key (`E` by default). Returns true iff anything interesting happened as a result.
- `bool onInteractNextTo(const PlayerPtr &)`: called when the player is standing one square from your tile entity facing your tile entity and presses the interaction key (`e` by default). Returns true iff anything interesting happened as a result.
- `void onOverlap(const EntityPtr &)`: called when an [entity](Entities.md) steps on the tile entity's position.

### Rendering

By default, tile entities are invisible and do nothing in their `render` method. If you want to render a tile from the tilemap, check out [Teleporter::render](https://github.com/heimskr/game3/blob/master/src/tileentity/Teleporter.cpp) for an example. <!-- TODO: move Teleporter rendering code into a protected utility function under TileEntity? -->

### Registering Your Tile Entity

<!-- TODO: after the plugin system is implemented, add instructions for plugins -->
In `Game3::Game::addTileEntityFactories`, add a line like this:
```c++
add(TileEntityFactory::create<MyTileEntity>());
```
This is for base-game tile entities. Once the plugin system is ready, instructions will be added here on how to register the tile entity from a plugin.

Once you've done all that, your new tile entity should be good to go!
