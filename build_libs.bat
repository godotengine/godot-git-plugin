git submodule init
git submodule update --init --recursive

cd godot-git-plugin\thirdparty\libgit2\
mkdir build
cd build\
del /F CMakeCache.txt
cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DBUILD_SHARED_LIBS=OFF -DBUILD_CLAR=OFF -DBUILD_EXAMPLES=OFF -DUSE_SSH=OFF -DUSE_HTTPS=OFF -DUSE_BUNDLED_ZLIB=ON -DWINHTTP=OFF
cmake --build . --config %1
cd ../../../../
mkdir "demo/bin/x11/"
copy godot-git-plugin\thirdparty\libgit2\build\%1\git2.lib demo\bin\win64\

if "%CI%"=="" (
    echo Non-CI build detected
    set SCONS=scons
) else (
    echo CI build detected
    set SCONS=..\scons.bat
)

cd godot-cpp\
%SCONS% platform=windows target=%1 generate_bindings=yes bits=64
cd ..
