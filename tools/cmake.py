import os
import shlex
import sys

import SCons.Action
import SCons.Builder
import SCons.Util


# This must be kept in sync with the value in https://github.com/godotengine/godot/blob/master/platform/android/detect.py#L58.
def get_ndk_version():
    return "23.2.8568313"


def cmake_default_flags(env):
    if env.get("cmake_default_flags", ""):
        return shlex.split(env["cmake_default_flags"])

    config = {}

    if "CC" in env:
        config["CMAKE_C_COMPILER"] = env["CC"]
    if "CXX" in env:
        config["CMAKE_CXX_COMPILER"] = env["CXX"]

    if env["platform"] == "android":
        api = env["android_api_level"]
        abi = {
            "arm64": "arm64-v8a",
            "arm32": "armeabi-v7a",
            "x86_32": "x86",
            "x86_64": "x86_64",
        }[env["arch"]]
        config["CMAKE_SYSTEM_NAME"] = "Android"
        config["CMAKE_SYSTEM_VERSION"] = api
        config["CMAKE_ANDROID_ARCH_ABI"] = abi
        config["ANDROID_ABI"] = abi
        ndk_root = os.environ.get("ANDROID_NDK_ROOT", env.get("ANDROID_HOME", "") + "/ndk/" + get_ndk_version())
        config["CMAKE_TOOLCHAIN_FILE"] = "%s/build/cmake/android.toolchain.cmake" % ndk_root
        config["CMAKE_ANDROID_STL_TYPE"] = "c++_static"

    elif env["platform"] == "linux":
        linux_flags = {
            "x86_64": "-m64",
            "x86_32": "-m32",
            "arm32": "-march=armv7-a",
            "arm64": "-march=armv8-a",
        }.get(env["arch"], "")
        if linux_flags:
            config["CMAKE_C_FLAGS"] = linux_flags
            config["CMAKE_CXX_FLAGS"] = linux_flags

    elif env["platform"] == "macos":
        if env["arch"] == "universal":
            config["CMAKE_OSX_ARCHITECTURES"] = '"x86_64;arm64"'
        else:
            config["CMAKE_OSX_ARCHITECTURES"] = env["arch"]
        if env.get("macos_deployment_target", "default") != "default":
            config["CMAKE_OSX_DEPLOYMENT_TARGET"] = env["macos_deployment_target"]
        if sys.platform != "darwin" and "OSXCROSS_ROOT" in os.environ:
            config["CMAKE_AR"] = env["AR"]
            config["CMAKE_RANLIB"] = env["RANLIB"]
            if env["arch"] == "universal":
                flags = "-arch x86_64 -arch arm64"
            else:
                flags = "-arch " + env["arch"]
            if env["macos_deployment_target"] != "default":
                flags += " -mmacosx-version-min=" + env["macos_deployment_target"]
            config["CMAKE_C_FLAGS"] = flags
            config["CMAKE_CXX_FLAGS"] = flags

    elif env["platform"] == "ios":
        if env["arch"] == "universal":
            raise ValueError("iOS architecture not supported: %s" % env["arch"])
        config["CMAKE_SYSTEM_NAME"] = "iOS"
        config["CMAKE_OSX_ARCHITECTURES"] = env["arch"]
        if env.get("ios_min_version", "default") != "default":
            config["CMAKE_OSX_DEPLOYMENT_TARGET"] = env["ios_min_version"]
        if env["ios_simulator"]:
            config["CMAKE_OSX_SYSROOT"] = "iphonesimulator"

    elif env["platform"] == "windows":
        config["CMAKE_SYSTEM_NAME"] = "Windows"
        if env.get("is_msvc", False):
            config["CMAKE_POLICY_DEFAULT_CMP0091"] = "NEW"
            if env.get("debug_crt", False):
                config["CMAKE_MSVC_RUNTIME_LIBRARY"] = "MultiThreadedDebugDLL"
            else:
                if env.get("use_static_cpp", False):
                    config["CMAKE_MSVC_RUNTIME_LIBRARY"] = "MultiThreaded"
                else:
                    config["CMAKE_MSVC_RUNTIME_LIBRARY"] = "MultiThreadedDLL"

    return ["-D%s=%s" % it for it in config.items()]


def cmake_emitter(target, source, env):
    return [str(target[0]) + "/CMakeCache.txt"] + target[1:], [str(source[0]) + "/CMakeLists.txt"] + source[1:]


def cmake_generator(target, source, env, for_signature):
    # Strip the -j option for signature to avoid rebuilding when num_jobs changes.
    build = env["CMAKEBUILDCOM"].replace("-j$CMAKEBUILDJOBS", "") if for_signature else env["CMAKEBUILDCOM"]
    actions = [
        SCons.Action.Action("$CMAKECONFCOM", "$CMAKECONFCOMSTR"),
        SCons.Action.Action(build, "$CMAKEBUILDCOMSTR"),
    ]
    if env["CMAKE_INSTALL"]:
        actions.append(
            SCons.Action.Action("$CMAKEINSTALLCOM", "$CMAKEINSTALLCOMSTR"),
        )
    return actions


def cmake_build(
    env, target_dir, source_dir, cmake_outputs=[], cmake_targets=[], cmake_options=[], dependencies=[], install=False
):
    cmake_env = env.Clone()
    target = env.Dir("{}/{}/{}".format(target_dir, env["platform"], env["arch"]))
    source = env.Dir(source_dir)
    builder_targets = [target] + [str(target) + "/" + f for f in cmake_outputs]
    builder_sources = [source] + dependencies
    default_flags = cmake_default_flags(env)

    # Merge flags
    flags = []
    for df in default_flags:
        if not df.startswith("-D"):
            flags.append(df)
        else:
            f = df[2:].split("=")[0]
            if f in cmake_options:
                df += " " + cmake_options[f]
                cmake_options.pop(f)
            flags.append(df)
    for opt in cmake_options:
        flags.append("-D%s=%s" % (opt, cmake_options[opt]))

    # Select generator
    if env["cmake_generator"]:
        flags.extend(["-G", env["cmake_generator"]])
    elif env["platform"] == "windows":
        if env.get("is_msvc", False):
            flags.extend(["-G", "NMake Makefiles"])
        elif sys.platform in ["win32", "msys", "cygwin"]:
            flags.extend(["-G", "Ninja"])
        else:
            flags.extend(["-G", "Unix Makefiles"])

    cmake_env.Append(CMAKECONFFLAGS=flags)
    if len(cmake_targets) > 0:
        cmake_env.Append(CMAKEBUILDFLAGS=["-t"] + [t for t in cmake_targets])
    cmake_env["CMAKE_INSTALL"] = install
    return cmake_env.CMake(builder_targets, builder_sources)


def options(opts):
    opts.Add("cmake_default_flags", "Default CMake platform flags override, will be autodetected if not specified.", "")
    opts.Add("cmake_generator", "CMake generator override, will be autodetected from platform if not specified.", "")
    opts.Add("cmake", "CMake binary to use", "cmake")


def exists(env):
    return True


def generate(env):
    env["CMAKE"] = env["cmake"]
    env["CMAKECONFFLAGS"] = SCons.Util.CLVar("")
    env["CMAKECONFCOM"] = "$CMAKE -B ${TARGET.dir} $CMAKECONFFLAGS ${SOURCE.dir}"
    env["CMAKEBUILDJOBS"] = "${__env__.GetOption('num_jobs')}"
    env["CMAKEBUILDFLAGS"] = SCons.Util.CLVar("")
    env["CMAKEINSTALLFLAGS"] = SCons.Util.CLVar("")
    env["CMAKEBUILDCOM"] = "$CMAKE --build ${TARGET.dir} $CMAKEBUILDFLAGS -j$CMAKEBUILDJOBS"
    env["CMAKEINSTALLCOM"] = "$CMAKE --install ${TARGET.dir} $CMAKEINSTALLFLAGS"
    env["BUILDERS"]["CMake"] = SCons.Builder.Builder(generator=cmake_generator, emitter=cmake_emitter)
    env.AddMethod(cmake_build, "CMakeBuild")
