# GDNative Based Git Plugin for Godot Version Control Editor Plugin
Implements the proxy end-points for the `EditorVCSInterface` API in the Godot Engine Editor. Uses [libgit2](https://libgit2.org) at its backend to simulate Git in code.

## Installation Instructions

 1. Plugin binary releases for Linux & Windows are here: <https://github.com/godotengine/godot-git-plugin/releases>
 2. Installation instructions are here: https://github.com/godotengine/godot-git-plugin/wiki

## Build Instructions

> Replace `Release` with `Debug` for a debug build.

### Windows
1. Open `build_libs.bat` as text.
2. Edit the relative paths to the Godot binary and from the Godot binary directory to this repository's directory in line 1 and line 2.
3. Load the x64 command prompt: `x64 Native Tools Command Prompt for VS 20XX`.
4. Run `build_libs.bat Release`.
5. Run `scons platform=windows target=release`

### Linux
1. Open `build_libs.sh` as text.
2. Edit the relative paths to the Godot binary and from the Godot binary directory to this repository's directory in line 1 and line 2.
3. Prepare script for execution: `chmod 755 build_libs.sh`
4. Run ```. ./build_libs.sh Release```.
5. Run `scons platform=x11 target=release`.
