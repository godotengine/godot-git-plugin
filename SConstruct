#!/usr/bin/env python

import os

EnsureSConsVersion(3, 0, 0)
EnsurePythonVersion(3, 5)

opts = Variables([], ARGUMENTS)

env = Environment(ENV = os.environ)

# Define our options
opts.Add(EnumVariable("target", "Compilation target", "debug", ["d", "debug", "r", "release"]))
opts.Add(EnumVariable("platform", "Compilation platform", "", ["", "windows", "linux", "osx"]))
opts.Add(EnumVariable("p", "Compilation target, alias for \"platform\"", "", ["", "windows", "linux", "osx"]))
opts.Add(BoolVariable("godot_cpp", "Build godot-cpp by forwarding arguments to it.", "no"))
opts.Add(BoolVariable("use_llvm", "Use the LLVM / Clang compiler", "no"))
opts.Add(PathVariable("target_path", "The path where the lib is installed.", "demo/addons/godot-git-plugin/"))
opts.Add(PathVariable("target_name", "The library name.", "libgitapi", PathVariable.PathAccept))
opts.Add(EnumVariable("bits", "The bit architecture.", "64", ["64"]))

# Updates the environment with the option variables.
opts.Update(env)

Export("env")

SConscript("thirdparty/SCsub")

if env["godot_cpp"]:
	if ARGUMENTS.get("use_custom_api_file", False) and ARGUMENTS.get("custom_api_file", "") != "":
		ARGUMENTS["custom_api_file"] = "../" + ARGUMENTS["custom_api_file"]

	SConscript("godot-cpp/SConstruct")

SConscript("godot-git-plugin/SCsub")

# Generates help for the -h scons option.
Help(opts.GenerateHelpText(env))
