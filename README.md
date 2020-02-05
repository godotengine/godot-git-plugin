# GDNative Based Git Plugin for Godot Version Control Editor Plugin
Implements the proxy end-points for the `EditorVCSInterface` API in the Godot Engine Editor. Uses [libgit2](https://libgit2.org) at its backend to simulate Git in code.

## Installation Instructions

 1. Plugin binary releases for Linux & Windows are here: <https://github.com/godotengine/godot-git-plugin/releases>
 2. Installation instructions are here: <https://godotengine.org/article/gsoc-2019-progress-report-3#vcs-integration>

## Build Instructions

### Windows
1. Open `build_libs_release.bat` as text.
2. Edit the relative paths to the Godot binary and from the Godot binary directory to this repository's directory in line 1 and line 2.
3. Run `build_libs_release.bat`.
4. Run `cd ..` because the build file leaves you one level deeper in the repository.
5. Load the x64 command prompt: `x64 Native Tools Command Prompt for VS 2017`.
6. Run `scons platform=windows target=release`

### Linux
1. Open `build_libs_release.sh` as text.
2. Edit the relative paths to the Godot binary and from the Godot binary directory to this repository's directory in line 1 and line 2.
3. Prepare script for execution: `chmod 755 build_libs_release.sh`
4. Run ```. ./build_libs_release.sh```.
5. Run `cd ..` because the build file leaves you one level deeper in the repository.
6. Run `scons platform=x11 target=release`.
