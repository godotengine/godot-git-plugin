#!/usr/bin/env python

import os
import platform

EnsureSConsVersion(3, 0, 0)
EnsurePythonVersion(3, 5)

opts = Variables([], ARGUMENTS)

env = Environment(ENV=os.environ)
aarch64 = "aarch" in platform.machine()

# Define our options
opts.Add(PathVariable("target_path",
         "The path where the lib is installed.", "demo/addons/godot-git-plugin/"))
opts.Add(PathVariable("target_name", "The library name.",
         "libgit_plugin", PathVariable.PathAccept))
opts.Add(PathVariable("macos_openssl", "Path to OpenSSL library root - only used in macOS builds.",
                      "/usr/local/opt/openssl@1.1/", PathVariable.PathAccept))  # TODO: Find a way to configure this to use the cloned OpenSSL source code, based on `macos_arch`.
opts.Add(PathVariable("macos_openssl_static_ssl", "Path to OpenSSL libssl.a library - only used in macOS builds.",
         os.path.join(os.path.abspath(os.getcwd()), "thirdparty/openssl/libssl.a"), PathVariable.PathAccept))
opts.Add(PathVariable("macos_openssl_static_crypto", "Path to OpenSSL libcrypto.a library - only used in macOS builds.",
         os.path.join(os.path.abspath(os.getcwd()), "thirdparty/openssl/libcrypto.a"), PathVariable.PathAccept))

if aarch64:
         opts.Add(PathVariable("linux_openssl_static_crypto", "Path to OpenSSL libcrypto.a library - only used in Linux builds.",
                  "/usr/lib/aarch64-linux-gnu/libcrypto.a", PathVariable.PathAccept))
         opts.Add(PathVariable("linux_openssl_static_ssl", "Path to OpenSSL libssl.a library - only used in Linux builds.",
                  "/usr/lib/aarch64-linux-gnu/libssl.a", PathVariable.PathAccept))
else:
         opts.Add(PathVariable("linux_openssl_static_ssl", "Path to OpenSSL libssl.a library - only used in Linux builds.",
                  "/usr/lib/x86_64-linux-gnu/libssl.a", PathVariable.PathAccept))
         opts.Add(PathVariable("linux_openssl_static_crypto", "Path to OpenSSL libcrypto.a library - only used in Linux builds.",
                  "/usr/lib/x86_64-linux-gnu/libcrypto.a", PathVariable.PathAccept))

# Updates the environment with the option variables.
opts.Update(env)

if ARGUMENTS.get("custom_api_file", "") != "":
    ARGUMENTS["custom_api_file"] = "../" + ARGUMENTS["custom_api_file"]

ARGUMENTS["target"] = "editor"
env = SConscript("godot-cpp/SConstruct").Clone()
opts.Update(env)

Export("env")

SConscript("thirdparty/SCsub")

SConscript("godot-git-plugin/SCsub")

# Generates help for the -h scons option.
Help(opts.GenerateHelpText(env))
