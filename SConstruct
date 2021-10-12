#!/usr/bin/env python

import os

EnsureSConsVersion(3, 0, 0)
EnsurePythonVersion(3, 5)

opts = Variables([], ARGUMENTS)

env = Environment(ENV=os.environ)

# Define our options
opts.Add(EnumVariable("target", "Compilation target",
         "debug", ["d", "debug", "r", "release"]))
opts.Add(EnumVariable("platform", "Compilation platform",
         "", ["", "windows", "linux", "osx"]))
opts.Add(EnumVariable("p", "Compilation target, alias for \"platform\"",
         "", ["", "windows", "linux", "osx"]))
opts.Add(BoolVariable(
    "godot_cpp", "Build godot-cpp by forwarding arguments to it.", "no"))
opts.Add(BoolVariable("use_llvm",
         "Use the LLVM / Clang compiler - only effective when targeting Linux or FreeBSD.", "no"))
opts.Add(PathVariable("target_path",
         "The path where the lib is installed.", "demo/addons/godot-git-plugin/"))
opts.Add(PathVariable("target_name", "The library name.",
         "libgitapi", PathVariable.PathAccept))
opts.Add(EnumVariable("bits", "The bit architecture.", "64", ["64"]))
opts.Add(EnumVariable("macos_arch", "Target macOS architecture",
         "universal", ["universal", "x86_64", "arm64"]))
opts.Add(PathVariable("macos_openssl", "Path to OpenSSL libraries - only used in macOS builds.",
         "/usr/local/opt/openssl@1.1/", PathVariable.PathAccept))
opts.Add(PathVariable("macos_openssl_static_ssl", "Path to OpenSSL libssl.a library - only used in macOS builds.",
         "/usr/local/opt/openssl@1.1/lib/libssl.a", PathVariable.PathAccept))
opts.Add(PathVariable("macos_openssl_static_crypto", "Path to OpenSSL libcrypto.a library - only used in macOS builds.",
         "/usr/local/opt/openssl@1.1/lib/libcrypto.a", PathVariable.PathAccept))

# Updates the environment with the option variables.
opts.Update(env)

if env["platform"] == "osx":
    # Use only clang on osx because we need to do universal builds
    env["CXX"] = "clang++"
    env["CC"] = "clang"

    if env["macos_arch"] == "universal":
        env.Append(LINKFLAGS=["-arch", "x86_64", "-arch", "arm64"])
        env.Append(CCFLAGS=["-arch", "x86_64", "-arch", "arm64"])
    else:
        env.Append(LINKFLAGS=["-arch", env["macos_arch"]])
        env.Append(CCFLAGS=["-arch", env["macos_arch"]])

Export("env")

SConscript("thirdparty/SCsub")

if env["godot_cpp"]:
    if ARGUMENTS.get("use_custom_api_file", False) and ARGUMENTS.get("custom_api_file", "") != "":
        ARGUMENTS["custom_api_file"] = "../" + ARGUMENTS["custom_api_file"]

    SConscript("godot-cpp/SConstruct")

SConscript("godot-git-plugin/SCsub")

# Generates help for the -h scons option.
Help(opts.GenerateHelpText(env))
