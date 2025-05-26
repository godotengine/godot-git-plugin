#!/usr/bin/env python

import os

EnsureSConsVersion(3, 0, 0)
EnsurePythonVersion(3, 5)

opts = Variables([], ARGUMENTS)

env = Environment(ENV=os.environ)

# Define our options
opts.Add(PathVariable("target_path",
         "The path where the lib is installed.", "demo/addons/godot-git-plugin/"))
opts.Add(PathVariable("target_name", "The library name.",
         "libgit_plugin", PathVariable.PathAccept))

# Updates the environment with the option variables.
opts.Update(env)

if ARGUMENTS.get("custom_api_file", "") != "":
    ARGUMENTS["custom_api_file"] = "../" + ARGUMENTS["custom_api_file"]

ARGUMENTS["target"] = "editor"
env = SConscript("godot-cpp/SConstruct").Clone()
env.PrependENVPath("PATH", os.getenv("PATH"))  # Prepend PATH, done upstream in recent godot-cpp verions.

# OpenSSL Builder
env.Tool("openssl", toolpath=["tools"])

# SSH2 Builder
env.Tool("cmake", toolpath=["tools"])
env.Tool("ssh2", toolpath=["tools"])
env.Tool("git2", toolpath=["tools"])

opts.Update(env)

ssl = env.OpenSSL()
ssh2 = env.BuildSSH2(ssl)
ssl += ssh2
git2 = env.BuildGIT2(ssl)

Export("ssl")
Export("env")

SConscript("godot-git-plugin/SCsub")

# Generates help for the -h scons option.
Help(opts.GenerateHelpText(env))
