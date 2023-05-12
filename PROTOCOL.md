# Packets

Packets are encoded as a little-endian 2-byte integer representing the packet type, followed by a little-endian 4-byte integer representing the packet length.

<!-- TODO: message encoding -->

## Packet Types

0. Not used.

1. **Protocol Version**: informs the other side of the protocol version in use.

	- `u32` Version

2. **Tile Entity**: informs a client of a tile entity's data.

	- `u64` Tile Entity ID
	- `string` Identifier
	- `i32` Realm ID
	- `...` Tile Entity Data

3. **Chunk Request**: asks the server to send a Chunk Tiles packet for a given chunk.

	- `i32` Realm ID
	- `i32[]` Chunk Positions

	The chunk positions will be sent as interlaced (x, y) pairs.

4. **Tile Update**: informs the client of the new tile ID for a single tile.

	- `i32` Realm ID
	- `u8` Layer
	- `{i64,i64}` Position
	- `u16` Tile ID

5. **Command Result**: informs a client of the result of a command.

	- `u64` Command ID
	- `bool` Success
	- `string` Message

6. **Command**: sent to the server to run a command.

	- `u64` Command ID
	- `string` Command

7. **Self Teleported**: tells a client the position of their player.

	- `i32` Realm ID
	- `{i64,i64}` Position

8. **Chunk Tiles**: sends all the terrain data for a chunk to a client.

	- `i32` Realm ID
	- `{i32,i32}` Chunk Position
	- `i16[3*64**2]` Tile IDs (layer 1, then 2, then 3)

	<!-- TODO: compression -->

9. **Realm Notice**: informs a client of the existence of a realm.

	- `i32` Realm ID
	- `string` Realm Type
	- `bool` Outdoors

10. **Login**: sent by a client to log into the server.

	- `string` Username
	- `u64` Token

11. **Login Status**: informs a client of the result of a login attempt.

	- `bool` Success
	- `string` Display Name

12. **Register Player**: sent by a client to register a new player account.

	- `string` Username
	- `string` Display Name

13. **Registration Status**: informs a client of the status of their registration attempt.

	- `u64` Token

	The token will be 0 if the attempt failed, or an arbitrary nonzero value otherwise.
