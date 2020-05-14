git submodule init
git submodule update --init --recursive

cd godot-git-plugin\thirdparty\libgit2\
mkdir build
cd build\
del /F CMakeCache.txt
cmake ..
cmake --build .
cd ../../../../
copy godot-git-plugin\thirdparty\libgit2\build\%1\git2.lib demo\bin\win64\

cd godot-cpp\
..\scons.bat platform=windows target=%1 generate_bindings=yes bits=64
cd ..
