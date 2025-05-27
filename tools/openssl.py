import os, sys
import SCons.Util
import SCons.Builder
import SCons.Action
from SCons.Defaults import Mkdir
from SCons.Variables import PathVariable, BoolVariable


def ssl_platform_target(env):
    targets = {}
    platform = env["platform"]
    if platform == "linux":
        targets = {
            "x86_32": "linux-x86",
            "x86_64": "linux-x86_64",
            "arm64": "linux-aarch64",
            "arm32": "linux-armv4",
            "rv64": "linux64-riscv64",
        }
    elif platform == "android":
        targets = {
            "arm64": "android-arm64",
            "arm32": "android-arm",
            "x86_32": "android-x86",
            "x86_64": "android-x86_64",
        }
    elif platform == "macos":
        targets = {
            "x86_64": "darwin64-x86_64",
            "arm64": "darwin64-arm64",
        }
    elif platform == "ios":
        if env["ios_simulator"]:
            targets = {
                "x86_64": "iossimulator-xcrun",
                "arm64": "iossimulator-xcrun",
            }
        else:
            targets = {
                "arm64": "ios64-xcrun",
                "arm32": "ios-xcrun",
            }
    elif platform == "windows":
        if env.get("is_msvc", False):
            targets = {
                "x86_32": "VC-WIN32",
                "x86_64": "VC-WIN64A",
            }
        else:
            targets = {
                "x86_32": "mingw",
                "x86_64": "mingw64",
            }

    arch = env["arch"]
    target = targets.get(arch, "")
    if target == "":
        raise ValueError("Architecture '%s' not supported for platform: '%s'" % (arch, platform))
    return target


def ssl_platform_options(env):
    ssl_config_options = [
        "no-ssl2",
        "no-ssl3",
        "no-weak-ssl-ciphers",
        "no-legacy",
        "no-shared",
        "no-tests",
    ]
    if env["platform"] == "windows":
        ssl_config_options.append("enable-capieng")
    return ssl_config_options


def ssl_platform_flags(env):
    args = []
    if env["platform"] == "android":
        if env.get("android_api_level", ""):
            api = int(env["android_api_level"])
            args.append("-D__ANDROID_API__=%s" % api)
    elif env["platform"] == "macos":
        if env["macos_deployment_target"] != "default":
            args.append("-mmacosx-version-min=%s" % env["macos_deployment_target"])
        # OSXCross toolchain setup.
        if sys.platform != "darwin" and "OSXCROSS_ROOT" in os.environ:
            for k in ["CC", "CXX", "AR", "AS", "RANLIB"]:
                args.append("%s=%s" % (k, env[k]))
    elif env["platform"] == "windows":
        is_win_host = sys.platform in ["win32", "msys", "cygwin"]
        if not (is_win_host or env.get("is_msvc", False)):
            mingw_prefixes = {
                "x86_32": "--cross-compile-prefix=i686-w64-mingw32-",
                "x86_64": "--cross-compile-prefix=x86_64-w64-mingw32-",
            }
            args.append(mingw_prefixes[env["arch"]])
    return args


def ssl_configure_args(env):
    if env.get("openssl_configure_options", ""):
        opts = SCons.Util.CLVar(env["openssl_configure_options"])
    else:
        opts = ssl_platform_options(env)

    if env.get("openssl_configure_target", ""):
        target = [env["openssl_configure_target"]]
    else:
        target = [ssl_platform_target(env)]

    if env.get("openssl_configure_flags", ""):
        flags = SCons.Util.CLVar(env["openssl_configure_flags"])
    else:
        flags = ssl_platform_flags(env)

    return opts + target + flags


def ssl_emitter(target, source, env):
    return env["SSL_LIBS"], [env.File(env["SSL_SOURCE"] + "/Configure"), env.File(env["SSL_SOURCE"] + "/VERSION.dat")]


def build_openssl(env, jobs=None):
    if env["SSL_EXTERNAL"]:
        # Setup the env to use the provided libraries, and return them without building.
        env.Prepend(CPPPATH=[env["SSL_INCLUDE"]])
        env.Prepend(LIBPATH=[env["SSL_BUILD"]])
        if env["platform"] == "windows":
            env.PrependUnique(LIBS=["crypt32", "ws2_32", "advapi32", "user32"])
        env.Prepend(LIBS=env["SSL_LIBS"])
        return [env["SSL_CRYPTO_LIBRARY"], env["SSL_LIBRARY"]]

    if jobs is None:
        jobs = int(env.GetOption("num_jobs"))

    # Since the OpenSSL build system does not support macOS universal binaries, we first need to build the two libraries
    # separately, then we join them together using lipo.
    if env["platform"] == "macos" and env["arch"] == "universal":
        build_envs = {
            "x86_64": env.Clone(),
            "arm64": env.Clone(),
        }
        arch_ssl = []
        for arch in build_envs:
            benv = build_envs[arch]
            benv["arch"] = arch
            generate(benv)
            benv["SSLBUILDJOBS"] = max([1, int(jobs / len(build_envs))])
            ssl = benv.OpenSSLBuilder()
            arch_ssl.extend(ssl)
            benv.NoCache(ssl)  # Needs refactoring to properly cache generated headers.

        # x86_64 and arm64 includes are equivalent.
        env["SSL_INCLUDE"] = build_envs["arm64"]["SSL_INCLUDE"]

        # Join libraries using lipo.
        lipo_action = "lipo $SOURCES -create -output $TARGET"
        ssl_libs = list(map(lambda arch: build_envs[arch]["SSL_LIBRARY"], build_envs))
        ssl_crypto_libs = list(map(lambda arch: build_envs[arch]["SSL_CRYPTO_LIBRARY"], build_envs))
        ssl = env.Command(env["SSL_LIBRARY"], ssl_libs, lipo_action)
        ssl += env.Command(env["SSL_CRYPTO_LIBRARY"], ssl_crypto_libs, lipo_action)
        env.Depends(ssl, arch_ssl)
    else:
        benv = env.Clone()
        benv["SSLBUILDJOBS"] = jobs
        ssl = benv.OpenSSLBuilder()
        benv.NoCache(ssl)  # Needs refactoring to properly cache generated headers.

    # Setup the environment to use the freshly built openssl.
    env.Prepend(CPPPATH=[env["SSL_INCLUDE"]])
    env.Prepend(LIBPATH=[env["SSL_BUILD"]])
    if env["platform"] == "windows":
        env.PrependUnique(LIBS=["crypt32", "ws2_32", "advapi32", "user32"])
    env.Prepend(LIBS=env["SSL_LIBS"])

    return ssl


def ssl_generator(target, source, env, for_signature):
    # Strip the -j option for signature to avoid rebuilding when num_jobs changes.
    build = env["SSLBUILDCOM"].replace("-j$SSLBUILDJOBS", "") if for_signature else env["SSLBUILDCOM"]
    return [
        Mkdir("$SSL_BUILD"),
        Mkdir("$SSL_INSTALL"),
        SCons.Action.Action("$SSLCONFIGCOM", "$SSLCONFIGCOMSTR"),
        SCons.Action.Action(build, "$SSLBUILDCOMSTR"),
    ]


def options(opts):
    opts.Add(PathVariable("openssl_source", "Path to the openssl sources.", "thirdparty/openssl"))
    opts.Add("openssl_build", "Destination path of the openssl build.", "bin/thirdparty/openssl")
    opts.Add(
        "openssl_configure_options",
        "OpenSSL configure options override. Will use a reasonable default if not specified.",
        "",
    )
    opts.Add(
        "openssl_configure_target", "OpenSSL configure target override, will be autodetected if not specified.", ""
    )
    opts.Add(
        "openssl_configure_flags",
        "OpenSSL configure compiler flags override. Will be autodetected if not specified.",
        "",
    )
    opts.Add(
        "openssl_external_crypto",
        'An external libcrypto static library (e.g. "/usr/lib/x86_64-linux-gnu/libcrypto.a"). If not provided, OpenSSL will be built from source.',
        "",
    )
    opts.Add(
        "openssl_external_ssl",
        'An external libssl static library (e.g. "/usr/lib/x86_64-linux-gnu/libssl.a"). If not provided, OpenSSL will be built from source.',
        "",
    )
    opts.Add(
        "openssl_external_include",
        'An external OpenSSL "include" folder (e.g. "/usr/include/openssl").',
        "",
    )


def exists(env):
    return True


def generate(env):
    env.AddMethod(build_openssl, "OpenSSL")

    # Check if the user specified infos about external OpenSSL files.
    external_opts = ["openssl_external_crypto", "openssl_external_ssl", "openssl_external_include"]
    is_set = lambda k: env.get(k, "") != ""
    if any(map(is_set, external_opts)):
        # Need provide the whole (crypto, ssl, include) triple to proceed.
        if not all(map(is_set, external_opts)):
            print('Error: The options "%s" must all be set to use a external library.' % '", "'.join(external_opts))
            sys.exit(255)

        env["SSL_CRYPTO_LIBRARY"] = env.File("${openssl_external_crypto}")
        env["SSL_LIBRARY"] = env.File("${openssl_external_ssl}")
        env["SSL_BUILD"] = env.Dir("${SSL_LIBRARY.dir}").abspath
        env["SSL_INSTALL"] = env.Dir("${SSL_LIBRARY.dir}").abspath
        env["SSL_INCLUDE"] = env.Dir("${openssl_external_include}").abspath
        env["SSL_LIBS"] = [env["SSL_LIBRARY"], env["SSL_CRYPTO_LIBRARY"]]
        env["SSL_EXTERNAL"] = True
        return

    # We will need to build our own OpenSSL library.
    env["SSL_EXTERNAL"] = False

    # Android needs the NDK in ENV, and proper PATH setup.
    if env["platform"] == "android" and env["ENV"].get("ANDROID_NDK_ROOT", "") == "":
        cc_path = os.path.dirname(env["CC"])
        if cc_path and cc_path not in env["ENV"]:
            env.PrependENVPath("PATH", cc_path)
        if "ANDROID_NDK_ROOT" not in env["ENV"]:
            env["ENV"]["ANDROID_NDK_ROOT"] = env.get("ANDROID_NDK_ROOT", os.environ.get("ANDROID_NDK_ROOT", ""))

    env["SSL_SOURCE"] = env.Dir(env["openssl_source"]).abspath
    env["SSL_BUILD"] = env.Dir(env["openssl_build"] + "/{}/{}".format(env["platform"], env["arch"])).abspath
    env["SSL_INSTALL"] = env.Dir(env["SSL_BUILD"] + "/dest").abspath
    env["SSL_INCLUDE"] = env.Dir(env["SSL_INSTALL"] + "/include").abspath
    lib_ext = ".lib" if env.get("is_msvc", False) else ".a"
    env["SSL_LIBRARY"] = env.File(env["SSL_BUILD"] + "/libssl" + lib_ext)
    env["SSL_CRYPTO_LIBRARY"] = env.File(env["SSL_BUILD"] + "/libcrypto" + lib_ext)
    env["SSL_LIBS"] = [env["SSL_LIBRARY"], env["SSL_CRYPTO_LIBRARY"]]

    # Configure action
    env["PERL"] = env.get("PERL", "perl")
    env["_ssl_configure_args"] = ssl_configure_args
    env["SSLPLATFORMCONFIG"] = "${_ssl_configure_args(__env__)}"
    env["SSLCONFFLAGS"] = SCons.Util.CLVar("")
    # fmt: off
    env["SSLCONFIGCOM"] = 'cd ${TARGET.dir} && $PERL -- ${SOURCE.abspath} --prefix="${SSL_INSTALL}" --openssldir="${SSL_INSTALL}" $SSLPLATFORMCONFIG $SSLCONFFLAGS'
    # fmt: on

    # Build action
    env["SSLBUILDJOBS"] = "${__env__.GetOption('num_jobs')}"
    # fmt: off
    env["SSLBUILDCOM"] = "make -j$SSLBUILDJOBS -C ${TARGET.dir} && make -j$SSLBUILDJOBS -C ${TARGET.dir} install_sw install_ssldirs"
    # fmt: on

    # Windows MSVC needs to build using NMake
    if env["platform"] == "windows" and env.get("is_msvc", False):
        env["SSLBUILDCOM"] = "cd ${TARGET.dir} && nmake install_sw install_ssldirs"

    env["BUILDERS"]["OpenSSLBuilder"] = SCons.Builder.Builder(generator=ssl_generator, emitter=ssl_emitter)
    env.AddMethod(build_openssl, "OpenSSL")
