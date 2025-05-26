import os


def build_library(env, deps):
    config = {
        "CMAKE_BUILD_TYPE": "RelWithDebInfo" if env["debug_symbols"] else "Release",
        "MbedTLS_LIBRARY": env["MBEDTLS_LIBRARY"],
        "MBEDCRYPTO_LIBRARY": env["MBEDTLS_CRYPTO_LIBRARY"],
        "MBEDX509_LIBRARY": env["MBEDTLS_X509_LIBRARY"],
        "MBEDTLS_INCLUDE_DIR": env["MBEDTLS_INCLUDE"],
        "BUILD_EXAMPLES": 0,
        "BUILD_TESTING": 0,
        "BUILD_SHARED_LIBS": 0,
        "CRYPTO_BACKEND": "mbedTLS",
    }

    if env["platform"] != "windows":
        config["CMAKE_C_FLAGS"] = "-fPIC"

    is_msvc = env.get("is_msvc", False)
    lib_ext = ".lib" if is_msvc else ".a"
    libs = ["src/libssh2{}".format(lib_ext)]

    source = env.Dir("#thirdparty/ssh2/libssh2").abspath
    target = env.Dir("#bin/thirdparty/libssh2").abspath

    ssh2 = env.CMakeBuild(
        "#bin/thirdparty/ssh2/",
        "#thirdparty/ssh2/libssh2",
        cmake_options=config,
        cmake_outputs=libs,
        cmake_targets=[],
        dependencies=deps,
    )

    env.Append(CPPPATH=["#thirdparty/ssh2/libssh2/include"])
    env.Prepend(LIBS=ssh2[1:])

    return ssh2


def exists(env):
    return "CMake" in env


def generate(env):
    env.AddMethod(build_library, "BuildSSH2")
