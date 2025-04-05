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

if [ ! -f "$ZIP" ]; then
	echo "\"$ZIP\" doesn't exist."
	exit 3
fi

scp "$ZIP" "$GAME3_REMOTE/$ZIP"
