#!/usr/bin/env python

import os

EnsureSConsVersion(3, 0, 0)
EnsurePythonVersion(3, 5)

opts = Variables([], ARGUMENTS)

env = Environment()

# Updates the environment with the option variables.
opts.Update(env)

if ARGUMENTS.get("custom_api_file", "") != "":
    ARGUMENTS["custom_api_file"] = "../" + ARGUMENTS["custom_api_file"]

ARGUMENTS["target"] = "editor"
env = SConscript("godot-cpp/SConstruct").Clone()

# Prepend PATH to allow using custom toolchains, done upstream in recent godot-cpp verions.
env.PrependENVPath("PATH", os.getenv("PATH"))

if env["platform"] == "windows" and env.get("is_msvc", False):
    # Force linking with LTO on windows MSVC, silence the linker complaining that libgit uses LTO but we are not linking with it.
    env.AppendUnique(LINKFLAGS=["/LTCG"])
    # Do not treat empty pdb as errors.
    env.AppendUnique(LINKFLAGS=["/IGNORE:4099"])

env.Tool("cmake", toolpath=["tools"])
env.Tool("mbedtls", toolpath=["tools"])
env.Tool("ssh2", toolpath=["tools"])
env.Tool("git2", toolpath=["tools"])

mbedtls = env.BuildMbedTLS()
ssh2 = env.BuildSSH2(mbedtls)
mbedtls += ssh2
git2 = env.BuildGIT2(mbedtls)

env.Append(CPPPATH=[".", "src/"])
env.Append(CPPPATH=["#thirdparty/git2/libgit2/include/"])

lib_sources = Glob("src/*.cpp")
env.Depends(lib_sources, mbedtls)

library = env.SharedLibrary(
    target="bin/addons/godot-git-plugin/lib/libgit_plugin{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
    source=lib_sources
)
extension = env.InstallAs("bin/addons/godot-git-plugin/git_plugin.gdextension", "misc/git_plugin.gdextension")

Default(library + extension)
