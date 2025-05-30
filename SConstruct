#!/usr/bin/env python

import os

EnsureSConsVersion(3, 1, 2)
EnsurePythonVersion(3, 6)

opts = Variables([], ARGUMENTS)

env = Environment(ENV=os.environ)

# Define our options
opts.Add(PathVariable("target_path",
         "The path where the lib is installed.", "addons/godot-git-plugin/"))
opts.Add(PathVariable("target_name", "The library name.",
         "libgit_plugin", PathVariable.PathAccept))

# Updates the environment with the option variables.
opts.Update(env)

if ARGUMENTS.get("custom_api_file", "") != "":
    ARGUMENTS["custom_api_file"] = "../" + ARGUMENTS["custom_api_file"]

ARGUMENTS["target"] = "editor"
env = SConscript("godot-cpp/SConstruct").Clone()
env.PrependENVPath("PATH", os.getenv("PATH"))  # Prepend PATH, done upstream in recent godot-cpp verions.

# Force linking with LTO on windows MSVC, silence the linker complaining that libgit uses LTO but we are not linking with it.
if env["platform"] == "windows" and env.get("is_msvc", False):
    env.AppendUnique(LINKFLAGS=["/LTCG"])

env.Tool("cmake", toolpath=["tools"])
env.Tool("mbedtls", toolpath=["tools"])
env.Tool("ssh2", toolpath=["tools"])
env.Tool("git2", toolpath=["tools"])

opts.Update(env)

mbedtls = env.BuildMbedTLS()
ssh2 = env.BuildSSH2(mbedtls)
mbedtls += ssh2
git2 = env.BuildGIT2(mbedtls)

Export("mbedtls")
Export("env")

SConscript("godot-git-plugin/SCsub")

# Generates help for the -h scons option.
Help(opts.GenerateHelpText(env))
