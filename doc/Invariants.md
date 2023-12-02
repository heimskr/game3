# Invariants

- The 0th index of a tileset (which is the position at (0, 0)) must be a transparent tile.
- REALM_DIAMETER should be positive and odd.
- Please don't modify the stores of Entities and TileEntities while Realms are ticking.
- Entities that directly virtually inherit from Entity will **not** call `Entity::encode`/`Entity::decode` inside their `encode`/`decode` overloads; inheriting classes have to do that themselves.
- If a tile entity occupies a tile, the submerged and object layers of that tile must be empty.
- Plantable tiles should be on the submerged layer.
- A Realm's generateChunk method must increase the chunk's update counter if it's 0.
- There should be an active generation guard during calls to Biome::generate and Biome::postgen.
- Luck stats must be in the range [0, 10].