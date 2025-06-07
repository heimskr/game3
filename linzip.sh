#!/bin/bash

cd "$1" &&
cp "$2"/src/game3 ./game3_stripped &&
strip game3_stripped &&
rm -f game3-linux-x86_64.zip &&
mkdir -p game3_zip/Game3 &&
cd game3_zip &&
cp -r ../resources Game3/resources &&
cp -r ../gamedata Game3/gamedata &&
cp ../game3_stripped Game3/game3 &&
zip -r ../game3-linux-x86_64.zip Game3 &&
cd .. &&
rm -r game3_zip &&
rm -f game3_stripped
