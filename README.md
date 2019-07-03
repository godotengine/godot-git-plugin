# GDNative Based Git Plugin for Godot Version Control Editor Plugin

## Build Instructions

### Windows
1. Open `build.bat` as text.
2. Edit the relative paths to the Godot binary and from the Godot binary directory to this repository's directory in line 1 and line 2.
3. Run `build.bat`.
4. Run `cd ..` because the build file leaves you one level deeper in the repository.
5. Load the x64 command prompt: `x64 Native Tools Command Prompt for VS 2017`.
6. Run `scons platform=windows`

### Linux
1. Open `build.sh` as text.
2. Edit the relative paths to the Godot binary and from the Godot binary directory to this repository's directory in line 1 and line 2.
3. Prepare script for execution: `chmod 755 build.sh`
4. Run ```. ./build.sh```.
5. Run `cd ..` because the build file leaves you one level deeper in the repository.
6. Run `scons platform=x11`.
