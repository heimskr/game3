# Invariants

- The 0th index of a tileset (which is the position at (0, 0)) must be a transparent tile.
- REALM_DIAMETER should be positive and odd.
- Please don't modify the stores of Entities and TileEntities while Realms are ticking.
- Entities that directly virtually inherit from Entity will **not** call `Entity::encode`/`Entity::decode` inside their `encode`/`decode` overloads; inheriting classes have to do that themselves.
