git submodule init;
git submodule update --init --recursive;

cd godot-git-plugin/thirdparty/libgit2/
mkdir build
cd build/
rm CMakeCache.txt
cmake .. -DBUILD_SHARED_LIBS=OFF -DBUILD_CLAR=OFF -DBUILD_EXAMPLES=OFF -DUSE_SSH=ON -DUSE_HTTPS=ON -DUSE_BUNDLED_ZLIB=ON -DREGEX_BACKEND=builtin
cmake --build .
cd ../../../../
mv godot-git-plugin/thirdparty/libgit2/build/libgit2.a demo/bin/x11/libgit2.a

cd godot-cpp/;
scons platform=linux target=$1 generate_bindings=yes bits=64;
cd ..
