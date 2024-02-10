import os


def build_library(env, deps):
    config = {
        "CMAKE_BUILD_TYPE": "RelWithDebInfo" if env["debug_symbols"] else "Release",
        "OPENSSL_USE_STATIC_LIBS": 1,
        "OPENSSL_INCLUDE_DIR": env["SSL_INCLUDE"],
        "OPENSSL_SSL_LIBRARY": env["SSL_LIBRARY"],
        "OPENSSL_CRYPTO_LIBRARY": env["SSL_CRYPTO_LIBRARY"],
        "OPENSSL_ROOT_DIR": env["SSL_INSTALL"],
        "BUILD_TESTS": 0,
        "BUILD_CLI": 0,
        "BUILD_EXAMPLES": 0,
        "BUILD_FUZZERS": 0,
        "USE_SSH": 1,
        "USE_HTTPS": 1,
        "USE_SHA1": 1,
        "USE_BUNDLED_ZLIB": 1,
        "USE_HTTP_PARSER": "builtin",
        "REGEX_BACKEND": "builtin",
        "USE_HTTPS": "OpenSSL",
        "USE_SHA1": "OpenSSL",
        "BUILD_SHARED_LIBS": 0,
        "LINK_WITH_STATIC_LIBRARIES": 1,
        "LIBSSH2_INCLUDE_DIR": env.Dir("#thirdparty/ssh2/libssh2/include").abspath,
        "LIBSSH2_LIBRARY": deps[-1],
        "USE_WINHTTP": 0,
        "STATIC_CRT": 0,
    }

    if env["platform"] != "windows":
        config["CMAKE_C_FLAGS"] = "-fPIC"
    else:
        config["OPENSSL_ROOT_DIR"] = env["SSL_BUILD"]

    is_msvc = env.get("is_msvc", False)
    lib_ext = ".lib" if is_msvc else ".a"
    lib_prefix = "" if is_msvc else "lib"
    libs = ["{}git2{}".format(lib_prefix, lib_ext)]

    source = env.Dir("#thirdparty/git2/libgit2").abspath
    target = env.Dir("#bin/thirdparty/libgit2").abspath

    git2 = env.CMakeBuild(
        "#bin/thirdparty/git2/",
        "#thirdparty/git2/libgit2",
        cmake_options=config,
        cmake_outputs=libs,
        cmake_targets=[],
        dependencies=deps,
    )

    env.Append(CPPPATH=["#thirdparty/git2/libgit2/include"])
    env.Prepend(LIBS=git2[1:])

    return git2


def exists(env):
    return "CMake" in env


def generate(env):
    env.AddMethod(build_library, "BuildGIT2")
