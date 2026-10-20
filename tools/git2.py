def build_library(env, ssl, ssh2):
    config = {
        "CMAKE_BUILD_TYPE": "RelWithDebInfo" if env["debug_symbols"] else "Release",
        "CMAKE_C_STANDARD": "99",
        "MBEDTLS_LIBRARY": env["MBEDTLS_LIBRARY"],
        "MBEDCRYPTO_LIBRARY": env["MBEDTLS_CRYPTO_LIBRARY"],
        "MBEDX509_LIBRARY": env["MBEDTLS_X509_LIBRARY"],
        "MBEDTLS_INCLUDE_DIR": env["MBEDTLS_INCLUDE"],
        "BUILD_TESTS": "OFF",
        "BUILD_CLI": "OFF",
        "BUILD_EXAMPLES": "OFF",
        "BUILD_FUZZERS": "OFF",
        "USE_SSH": "libssh2",
        "USE_HTTPS": "mbedTLS",
        "USE_SHA1": "CollisionDetection",
        "USE_BUNDLED_ZLIB": "ON",
        "USE_HTTP_PARSER": "builtin",
        "REGEX_BACKEND": "builtin",
        "BUILD_SHARED_LIBS": 0,
        "LINK_WITH_STATIC_LIBRARIES": 1,
        "LIBSSH2_INCLUDE_DIRS": env.Dir("#thirdparty/ssh2/libssh2/include").abspath,
        "LIBSSH2_RESOLVED": ssh2[-1].abspath,
        "LIBSSH2_LIBRARIES": "LIBSSH2",
        "LIBSSH2_FOUND": 1,
        "CMAKE_POSITION_INDEPENDENT_CODE": "ON",
        "STATIC_CRT": env.get("use_static_cpp", True),
        "CMAKE_C_FLAGS": env.MbedTLSFlags(),
    }

    is_msvc = env.get("is_msvc", False)
    lib_ext = ".lib" if is_msvc else ".a"
    lib_prefix = "" if is_msvc else "lib"
    libs = ["{}git2{}".format(lib_prefix, lib_ext)]

    git2 = env.CMakeBuild(
        "#bin/thirdparty/git2/",
        "#thirdparty/git2/libgit2",
        cmake_options=config,
        cmake_outputs=libs,
        cmake_targets=[],
        dependencies=ssl + ssh2,
    )

    env.Append(CPPPATH=["#thirdparty/git2/libgit2/include"])
    env.Prepend(LIBS=git2[1:])

    if env["platform"] == "windows":
        env.AppendUnique(LIBS=["secur32"])
    elif env["platform"] == "macos":
        env.AppendUnique(LIBS=["iconv"])

    return git2


def exists(env):
    return "CMake" in env


def generate(env):
    env.AddMethod(build_library, "BuildGIT2")
