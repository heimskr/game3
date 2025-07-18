#!/bin/bash

# Usage: quasi_msys2.sh <game3_repo> <quasi_msys2_repo> [publish]

source "$2"/env/all.src &&
source "$2"/init.sh &&
cd "$1"/build_windows &&
sed -i 's/-I\/usr\/include //' build.ninja &&
ninja || { sed -i 's/-I\/usr\/include //' build.ninja && ninja; } &&
cp src/game3.exe ../game3.exe &&
cd .. &&
strip ./game3.exe &&
rm -f game3-windows-x86_64.zip &&
mkdir -p game3_windows_zip/Game3 &&
cd game3_windows_zip &&
cp -r ../resources Game3/resources &&
cp -r ../gamedata Game3/gamedata &&
cp ../game3.exe Game3/ &&
cp ../dlls/*.dll Game3/ &&
zip -r ../game3-windows-x86_64.zip Game3 &&
cd .. &&
rm -r game3_windows_zip && if [ "$3" = "publish" ]; then
	echo Publishing.
	./publish.sh windows
	exit $?
else
	echo Not publishing.
fi
exit $?
