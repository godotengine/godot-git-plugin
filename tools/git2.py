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
        "LIBSSH2_INCLUDE_DIR": env.Dir("#thirdparty/libssh2/include").abspath,
        "LIBSSH2_LIBRARY": deps[-1],
        "USE_WINHTTP": 0,
        "STATIC_CRT": env.get("use_static_cpp", True),
    }

    if env["platform"] != "windows":
        config["CMAKE_C_FLAGS"] = "-fPIC"
    elif env.get("is_msvc", False):
        # For some reasons, libgit2 doesn't respect the linker flags
        link_arch = {
            "x86_64": "X64",
            "x86_32": "X86",
            "arm64": "ARM64",
        }[env["arch"]]
        #config["CMAKE_STATIC_LIBRARY_OPTIONS"] = "/MACHINE:" + link_arch
        #config["CMAKE_STATIC_LIBRARY_FLAGS"] = "/MACHINE:" + link_arch
        #config["CMAKE_STATIC_LINKER_FLAGS"] = "/MACHINE:" + link_arch
        #config["CMAKE_MODULE_LINKER_FLAGS"] = "/MACHINE:" + link_arch
        #config["CMAKE_LINK_OPTIONS"] = "/MACHINE:" + link_arch
        #config["CMAKE_LINK_FLAGS"] = "/MACHINE:" + link_arch

    is_msvc = env.get("is_msvc", False)
    lib_ext = ".lib" if is_msvc else ".a"
    lib_prefix = "" if is_msvc else "lib"
    libs = ["{}git2{}".format(lib_prefix, lib_ext)]

    git2 = env.CMakeBuild(
        env.Dir("bin/thirdparty/libgit2/"),
        env.Dir("thirdparty/libgit2"),
        cmake_options=config,
        cmake_outputs=libs,
        cmake_targets=[],
        dependencies=deps,
    )

    env.Append(CPPPATH=[env.Dir("thirdparty/libgit2/include")])
    env.Prepend(LIBS=git2[1:])
    if env["platform"] == "windows":
        env.PrependUnique(LIBS=["secur32", "advapi32"])

    return git2


def exists(env):
    return "CMake" in env


def generate(env):
    env.AddMethod(build_library, "BuildGIT2")
