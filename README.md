[![C/C++ CI](https://github.com/godotengine/godot-git-plugin/actions/workflows/build.yml/badge.svg)](https://github.com/godotengine/godot-git-plugin/actions/workflows/build.yml)

# GDNative Based Git Plugin for Godot Version Control Editor Plugin

Git implementation of the Godot Engine VCS interface in Godot. We use [libgit2](https://libgit2.org) as our backend to simulate Git in code.

> Planned for the upcoming version of Godot. Look for other branches for support in other Godot releases.

## Installation Instructions

1.  Grab the platform binaries here: <https://github.com/godotengine/godot-git-plugin/releases>
2.  Then read the installation instructions: https://github.com/godotengine/godot-git-plugin/wiki

## Build Instructions

This section onwards is only meant to be used if you intend to compile the plugin from source.

### Required Tools

- [SCons](https://scons.org/pages/download.html) (v3.0.1+)
- C++17 and C90 compilers detectable by SCons and present in `PATH`.
- For Mac users, run `brew install openssl@1.1`.
- For Linux users, `sudo apt-get install libssl-dev`, or local package manager equivalent.

### Build

```
scons platform=<platform> target=release bits=64 -j 6
```

For more build options, run `scons platform=<platform> -h`

## Bleeding Edge

Most of the times when new featured are being worked on for the Godot VCS Integration, this requires making changes inside of the Godot Editor source code along with this plugin. Thus, this means we need to manually generate the GDNative API from these newer Godot builds and then use them with godot-cpp.

To build using custom GDNative API definition JSON files, run the below helper command:

```
scons platform=<platform> target=debug godot_cpp=yes generate_bindings=yes bits=64 use_custom_api_file=yes custom_api_file=path/to/api.json -j 6
```

Once this command can completed successfully, the standard build commands in the above section can be run without recompiling godot-cpp. To stop godot-cpp from recompiling, do not use the `godot_cpp` option in SCons arguments. To view more options available while recompiling godot-cpp, run `scons platform=<platform> godot_cpp=yes -h`
