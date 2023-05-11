# Packets

Packets are encoded as a little-endian 2-byte integer representing the packet type, followed by a little-endian 4-byte integer representing the packet length.

<!-- TODO: message encoding -->

## Packet Types

1. Protocol Version

	- `u32` Version

2. Tile Entity

	- `u64` Tile Entity ID
	- `string` Identifier
	- `i32` Realm ID
	- `...` Tile Entity Data

3. Chunk Request

	- `i32` Realm ID
	- `i32[]` Chunk Positions

	The chunk positions will be sent as interlaced (x, y) pairs.

4. Tile Update

	- `i32` Realm ID
	- `u8` Layer
	- `{i64,i64}` Position
	- `u16` Tile ID

5. Command Result

	- `u64` Command ID
	- `bool` Success
	- `string` Message

6. Command

	- `u64` Command ID
	- `string` Command

7. Self Teleported

	- `i32` Realm ID
	- `{i64,i64}` Position
