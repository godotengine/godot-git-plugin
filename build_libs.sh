GODOT_PATH_RELATIVE_TO_PLUGIN="../godot/bin";
GIT_PLUGIN_RELATIVE_TO_GODOT="../../godot-git-plugin/";
git submodule init;
git submodule update --init --recursive;

cd godot-git-plugin/thirdparty/libgit2/
mkdir build
cd build/
rm CMakeCache.txt
cmake ..
cmake --build .
cd ../../../../
copy godot-git-plugin/thirdparty/libgit2/build/$1/libgit2.a demo/bin/x11/libgit2.a

cd godot-cpp/;
scons platform=linux target=$1 generate_bindings=yes bits=64;
cd ..
