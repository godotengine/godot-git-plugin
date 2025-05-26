import os


def build_library(env, deps):
    config = {
        "CMAKE_BUILD_TYPE": "RelWithDebInfo" if env["debug_symbols"] else "Release",
        "CMAKE_C_STANDARD": "99",
        "USE_SHARED_MBEDTLS_LIBRARY": 0,
        "USE_STATIC_MBEDTLS_LIBRARY": 1,
        "MBEDTLS_LIBRARY": env["MBEDTLS_LIBRARY"],
        "MBEDCRYPTO_LIBRARY": env["MBEDTLS_CRYPTO_LIBRARY"],
        "MBEDX509_LIBRARY": env["MBEDTLS_X509_LIBRARY"],
        "MBEDTLS_INCLUDE_DIR": env["MBEDTLS_INCLUDE"],
        "BUILD_TESTS": 0,
        "BUILD_CLI": 0,
        "BUILD_EXAMPLES": 0,
        "BUILD_FUZZERS": 0,
        "USE_SSH": "libssh2",
        "USE_BUNDLED_ZLIB": 1,
        "USE_HTTP_PARSER": "builtin",
        "REGEX_BACKEND": "builtin",
        "USE_HTTPS": "mbedTLS",
        "USE_SHA1": "mbedTLS",
        "BUILD_SHARED_LIBS": 0,
        "LINK_WITH_STATIC_LIBRARIES": 1,
        "LIBSSH2_INCLUDE_DIR": env.Dir("#thirdparty/ssh2/libssh2/include").abspath,
        "LIBSSH2_LIBRARY": deps[-1],
        "USE_WINHTTP": 0,
        "STATIC_CRT": env.get("use_static_cpp", True),
        "CMAKE_DISABLE_FIND_PACKAGE_ZLIB": 1,
        "CMAKE_DISABLE_FIND_PACKAGE_OPENSSL": 1,
    }

    if env["platform"] != "windows":
        config["CMAKE_C_FLAGS"] = "-fPIC"

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
    if env["platform"] == "windows":
        env.PrependUnique(LIBS=["secur32"])

    return git2


def exists(env):
    return "CMake" in env


def generate(env):
    env.AddMethod(build_library, "BuildGIT2")
