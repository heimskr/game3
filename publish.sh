#!/bin/bash

TARGET="$1"

if [ -z "$TARGET" ]; then
	if [ -z "$GAME3_PLATFORM" ]; then
		echo "Please specify a target."
		exit 1
	else
		TARGET="$GAME3_PLATFORM"
	fi
fi

if [ -z "$GAME3_REMOTE" ]; then
	echo "Please set GAME3_REMOTE to username@host:path."
	exit 2
fi

ZIP="game3-$TARGET-x86_64.zip"
HASH="game3-$TARGET-x86_64.hash"
STAMP="game3-$TARGET-x86_64.stamp"

if [ ! -f "$ZIP" ]; then
	echo "\"$ZIP\" doesn't exist."
	exit 3
fi

if [ "$TARGET" = "windows" ]; then
	./game3.exe --hash > "$HASH"
else
	./game3 --hash > "$HASH"
fi

echo "Hash: $(cat "$HASH")"

printf "%s" "$(date -u +%s)" > "$STAMP"
echo "Timestamp: $(cat "$STAMP")"

scp "$ZIP" "$GAME3_REMOTE/$ZIP"
scp "$HASH" "$GAME3_REMOTE/$HASH"
scp "$STAMP" "$GAME3_REMOTE/$STAMP"
