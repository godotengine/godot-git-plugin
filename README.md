# GDNative Based Git Plugin for Godot Version Control Editor Plugin

Git implementation of the Godot Engine VCS interface in Godot 3.2+. We use [libgit2](https://libgit2.org) as our backend to simulate Git in code.

## Installation Instructions

 1. Grab the Linux and/or Windows binaries here: <https://github.com/godotengine/godot-git-plugin/releases>
 2. Then read the installation instructions: https://github.com/godotengine/godot-git-plugin/wiki

## Build Instructions

### Pre-requisites

Required build tools:

* [CMake](https://cmake.org/download/) (v3.5.1+)
* [SCons](https://scons.org/pages/download.html) (v3.0.1+)

### Windows

> MSVC is our recommended compiler for Windows

1. Load the x64 command prompt: `x64 Native Tools Command Prompt for VS 20XX`.
2. Run ```build_libs.bat Release```.
3. Run ```scons platform=windows target=release```

### Linux

> G++ is our recommended compiler for Linux

1. Prepare script for execution: ```chmod 755 build_libs.sh```
2. Run ```. ./build_libs.sh Release```.
3. Run ```scons platform=x11 target=release```.

#### Debug build

Replace `Release` with `Debug` and `release` with `debug` in the above instructions for a debug build. You will also have to do the same in the paths mentioned in `demo/git_api.gdnlib` before opening the demo project in Godot.
