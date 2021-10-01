#!/bin/sh
git submodule init;
git submodule update --init --recursive;

cd godot-git-plugin/thirdparty/libgit2/
mkdir build
cd build/
rm -f CMakeCache.txt
cmake .. -DCMAKE_C_FLAGS="-arch arm64 -arch x86_64" -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DBUILD_SHARED_LIBS=OFF -DBUILD_CLAR=OFF -DBUILD_EXAMPLES=OFF -DUSE_SSH=OFF -DUSE_HTTPS=OFF -DUSE_BUNDLED_ZLIB=ON -DUSE_ICONV=OFF
cmake --build . --config $1
cd ../../../../
mkdir -p "demo/addons/godot-git-plugin/osx/"
cp "godot-git-plugin/thirdparty/libgit2/build/libgit2.a" "demo/addons/godot-git-plugin/osx/libgit2.a"

if [ -z "$CI" ]
then
	echo "Non-CI run was detected"
else
	echo "CI run was detected"
fi

cd godot-cpp/;
CORES=$(grep -c ^processor /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu)
scons platform=osx target=$1 generate_bindings=yes macos_arch=x86_64 -j$CORES;
scons platform=osx target=$1 generate_bindings=yes macos_arch=arm64 -j$CORES;
shopt -s nocasematch; if [[ "release" =~ "$1" ]]; then
    lipo -create ./bin/libgodot-cpp.osx.release.64.a ./bin/libgodot-cpp.osx.release.arm64.a -output ./bin/libgodot-cpp.osx.release.universal.a
else
    lipo -create ./bin/libgodot-cpp.osx.debug.64.a ./bin/libgodot-cpp.osx.debug.arm64.a -output ./bin/libgodot-cpp.osx.debug.universal.a
fi
cd ..
