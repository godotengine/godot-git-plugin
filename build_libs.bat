git submodule init
git submodule update --init --recursive

cd godot-git-plugin\thirdparty\libgit2\
mkdir build
cd build\
del /F CMakeCache.txt
cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DBUILD_SHARED_LIBS=OFF -DBUILD_CLAR=OFF -DBUILD_EXAMPLES=OFF -DUSE_SSH=OFF -DUSE_HTTPS=OFF -DUSE_BUNDLED_ZLIB=ON -DWINHTTP=OFF
cmake --build . --config %1
cd ../../../../
mkdir "demo/addons/godot-git-plugin/win64/"
copy godot-git-plugin\thirdparty\libgit2\build\%1\git2.lib demo\addons\godot-git-plugin\win64\git2.lib

if "%CI%"=="" (
    echo Non-CI build detected
) else (
    echo CI build detected
)

cd godot-cpp\
scons platform=windows target=%1 generate_bindings=yes bits=64 -j%NUMBER_OF_PROCESSORS%
cd ..
