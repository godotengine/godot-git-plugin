def get_build_flags(env):
    flags = ""
    if env["MBEDTLS_CONFIG"]:
        if env.msvc:
            flags += ' \\"\\"-DMBEDTLS_CONFIG_FILE=<{}>\\"\\"'.format(env["MBEDTLS_CONFIG"])
        else:
            flags += " '-DMBEDTLS_CONFIG_FILE=\\\"{}\\\"'".format(env["MBEDTLS_CONFIG"])
    if env["MBEDTLS_THREADING_ALT"]:
        if env.msvc:
            flags += ' /I\\"\\"{}\\"\\"'.format(env.File(env["MBEDTLS_THREADING_ALT"]).dir.abspath)
        else:
            flags += " '-I{}'".format(env.File(env["MBEDTLS_THREADING_ALT"]).dir.abspath)
    return flags


def build_library(env):
    deps = []
    mbedtls_bin = env.Dir("bin/thirdparty/mbedtls/{}/{}/install".format(env["platform"], env["arch"]))
    c_flags = env.MbedTLSFlags()
    if env["MBEDTLS_CONFIG"]:
        if env.msvc:
            env.Append(CPPDEFINES=[("MBEDTLS_CONFIG_FILE", '"<{}>"'.format(env["MBEDTLS_CONFIG"]))])
        else:
            env.Append(CPPDEFINES=[("MBEDTLS_CONFIG_FILE", '\\"{}\\"'.format(env["MBEDTLS_CONFIG"]))])
        deps.append(env["MBEDTLS_CONFIG"])
    if env["MBEDTLS_THREADING_ALT"]:
        deps += [env.File(env["MBEDTLS_THREADING_ALT"])]

    if env["platform"] == "windows" and not env.msvc:
        c_flags += " -D__USE_MINGW_ANSI_STDIO=0"  # See https://github.com/Mbed-TLS/mbedtls/issues/10161

    mbedtls_config = {
        "CMAKE_BUILD_TYPE": "RelWithDebInfo" if env["debug_symbols"] else "Release",
        "ENABLE_TESTING": 0,
        "ENABLE_PROGRAMS": 0,
        "CMAKE_INSTALL_PREFIX": env.Dir(mbedtls_bin).abspath,
        "CMAKE_C_FLAGS": c_flags,
        "MBEDTLS_FATAL_WARNINGS": 0,
        "CMAKE_INSTALL_LIBDIR": "lib",
        "CMAKE_POSITION_INDEPENDENT_CODE": "ON",
    }

    lib_ext = ".lib" if env.msvc else ".a"
    lib_prefix = "" if env.msvc else "lib"
    mbedtls_libs = [
        "/install/lib/{}mbedtls{}".format(lib_prefix, lib_ext),
        "/install/lib/{}mbedx509{}".format(lib_prefix, lib_ext),
        "/install/lib/{}mbedcrypto{}".format(lib_prefix, lib_ext),
    ]

    mbedtls_cmake_config = [
        "/install/lib/cmake/MbedTLS/MbedTLSConfig.cmake",
        "/install/lib/cmake/MbedTLS/MbedTLSConfigVersion.cmake",
        "/install/lib/cmake/MbedTLS/MbedTLSTargets.cmake",
    ]

    # Build libdatachannel
    mbedtls = env.CMakeBuild(
        env.Dir("bin/thirdparty/mbedtls/"),
        env.Dir("thirdparty/mbedtls"),
        cmake_options=mbedtls_config,
        cmake_outputs=mbedtls_libs + mbedtls_cmake_config,
        dependencies=deps,
        install=True,
    )

    # Configure env.
    if env["platform"] == "windows":
        env.PrependUnique(LIBS=["bcrypt", "ws2_32", "iphlpapi"])
    if env["platform"] == "linux":
        env.PrependUnique(LIBS=["pthread"])
    env.Prepend(LIBS=list(filter(lambda f: str(f).endswith(lib_ext), mbedtls)))
    env.Append(CPPPATH=[env.Dir("thirdparty/mbedtls/include")])

    return mbedtls


def exists(env):
    return "CMake" in env


def generate(env):
    mbedtls_install_dir = "bin/thirdparty/mbedtls/{}/{}/install".format(env["platform"], env["arch"])
    lib_ext = ".lib" if env.msvc else ".a"
    mbedtls = env.File(mbedtls_install_dir + "/lib/libmbedtls" + lib_ext)
    crypto = env.File(mbedtls_install_dir + "/lib/libmbedcrypto" + lib_ext)
    x509 = env.File(mbedtls_install_dir + "/lib/libmbedx509" + lib_ext)
    includes = env.Dir("thirdparty/mbedtls/include")
    env["MBEDTLS_LIBRARY"] = mbedtls.abspath
    env["MBEDTLS_CRYPTO_LIBRARY"] = crypto.abspath
    env["MBEDTLS_X509_LIBRARY"] = x509.abspath
    env["MBEDTLS_INCLUDE"] = includes.abspath
    env["MBEDTLS_CONFIG"] = ""
    env["MBEDTLS_THREADING_ALT"] = ""
    env.AddMethod(build_library, "BuildMbedTLS")
    env.AddMethod(get_build_flags, "MbedTLSFlags")
