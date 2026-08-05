# Compiling from source

## Required tools

- **Full** copy of the source code. Remember to use `git clone --recursive`,
  or initialize submodules with `git submodule update --init` after cloning.
- [SCons](https://scons.org/pages/download.html) (v3.1.2+), CMake, and Perl.
- C++17 and C99 compilers detectable by SCons and present in `PATH`.

## Release build

```
scons platform=<platform> target=editor
```

> [!NOTE]
>
> You may get the GDExtension dump yourself from Godot using the instructions
> in the next section, or use the ones provided in [godot-cpp](https://github.com/godotengine/godot-cpp).

To see a full list of build options, run `scons platform=<platform> --help`.

## Development builds

When new features are being worked on for the Godot VCS Integration, the build
process sometimes requires developers to make changes in the GDExtension API along
with this plugin. This means we need to manually generate the GDExtension API
from the custom Godot builds and use it to compile godot-cpp,
and then finally link the resulting godot-cpp binary into this plugin.

If you need to use a custom GDExtension API:

1. Dump the new bindings from the custom Godot build.

```
path/to/godot/bin/godot.<platform>.editor.<arch> --headless --dump-gdextension-interface --dump-extension-api
```

2. Build the plugin along with the godot-cpp library.

```
scons platform=<platform> target=editor generate_bindings=yes dev_build=yes
```

> [!NOTE]
>
> You only need to build godot-cpp once every change in the GDExtension API.
> Hence, `generate_bindings=yes` should only be passed in during the first
> time after generating a new GDExtension API dump.

3. To test the plugin, set up a testing project with Godot, and copy or symlink
   the `addons` folder.

To view more options available while recompiling godot-git-plugin, run `scons platform=<platform> -h`.
