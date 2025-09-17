import os


def build_library(env, deps):
    config = {
        "CMAKE_BUILD_TYPE": "RelWithDebInfo" if env["debug_symbols"] else "Release",
        "OPENSSL_USE_STATIC_LIBS": 1,
        "OPENSSL_INCLUDE_DIR": env["SSL_INCLUDE"],
        "OPENSSL_SSL_LIBRARY": env["SSL_LIBRARY"].abspath,
        "OPENSSL_CRYPTO_LIBRARY": env["SSL_CRYPTO_LIBRARY"].abspath,
        "OPENSSL_ROOT_DIR": env["SSL_INSTALL"],
        "BUILD_EXAMPLES": 0,
        "BUILD_TESTING": 0,
        "BUILD_SHARED_LIBS": 0,
        "CMAKE_DISABLE_FIND_PACKAGE_ZLIB": 1,
        "CMAKE_DISABLE_FIND_PACKAGE_OPENSSL": 1,
        "CRYPTO_BACKEND": "OpenSSL",
        "CMAKE_POLICY_VERSION_MINIMUM": 3.5,
    }

    if env["platform"] != "windows":
        config["CMAKE_C_FLAGS"] = "-fPIC"
    else:
        config["OPENSSL_ROOT_DIR"] = env["SSL_BUILD"]

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
