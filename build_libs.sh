git submodule init;
git submodule update --init --recursive;

cd godot-git-plugin/thirdparty/libgit2/
mkdir build
cd build/
rm CMakeCache.txt
cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DBUILD_SHARED_LIBS=OFF -DBUILD_CLAR=OFF -DBUILD_EXAMPLES=OFF -DUSE_SSH=OFF -DUSE_HTTPS=OFF -DUSE_BUNDLED_ZLIB=ON
cmake --build . --config $1
cd ../../../../
mkdir -p "demo/bin/x11/"
cp "godot-git-plugin/thirdparty/libgit2/build/libgit2.a" "demo/bin/x11/libgit2.a"

if [ -z "$CI" ]
then
	echo "Non-CI run was detected"     
else
	echo "CI run was detected"
fi

cd godot-cpp/;
scons platform=linux target=$1 generate_bindings=yes bits=64;
cd ..
