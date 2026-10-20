#!/usr/bin/env python

import os

EnsureSConsVersion(3, 1, 2)
EnsurePythonVersion(3, 6)

opts = Variables([], ARGUMENTS)

env = Environment(ENV=os.environ)

# Define our options
opts.Add(
    PathVariable(
        "target_path",
        "The path where the lib is installed (will be created if it does not exists).",
        "addons/godot-git-plugin/",
        PathVariable.PathAccept,
    )
)
opts.Add(PathVariable("target_name", "The library name.", "libgit_plugin", PathVariable.PathAccept))

# Updates the environment with the option variables.
opts.Update(env)

if ARGUMENTS.get("custom_api_file", "") != "":
    ARGUMENTS["custom_api_file"] = "../" + ARGUMENTS["custom_api_file"]

ARGUMENTS["target"] = "editor"
env = SConscript("godot-cpp/SConstruct").Clone()
env.__class__.msvc = env.get("is_msvc", False)

# Force linking with LTO on windows MSVC, silence the linker complaining that libgit uses LTO but we are not linking with it.
if env["platform"] == "windows" and env.get("is_msvc", False):
    env.AppendUnique(LINKFLAGS=["/LTCG"])

env.Tool("cmake", toolpath=["tools"])
env.Tool("mbedtls", toolpath=["tools"])
env.Tool("ssh2", toolpath=["tools"])
env.Tool("git2", toolpath=["tools"])

opts.Update(env)

# Generates help for the -h scons option.
Help(opts.GenerateHelpText(env))

env["MBEDTLS_CONFIG"] = env.File("godot-git-plugin/include/std_mbedtls_config.h").abspath
env["MBEDTLS_THREADING_ALT"] = env.File("godot-git-plugin/include/threading_alt.h").abspath

ssl = env.BuildMbedTLS()
ssh2 = env.BuildSSH2(ssl)
git2 = env.BuildGIT2(ssl, ssh2)

# Build our sources
env.Append(CPPPATH=["godot-git-plugin/include/", "godot-git-plugin/src/"])

env.Append(CPPPATH=["#thirdparty/git2/libgit2/include/"])

lib_sources = Glob("godot-git-plugin/src/*.cpp")
env.Depends(lib_sources, ssl + ssh2)
library = env.SharedLibrary(
    target=env["target_path"] + "/lib/{}{}{}".format(env["target_name"], env["suffix"], env["SHLIBSUFFIX"]),
    source=lib_sources,
)
library += env.InstallAs(env["target_path"] + "/git_plugin.gdextension", env.File("#misc/git_plugin.gdextension"))
Default(library)
