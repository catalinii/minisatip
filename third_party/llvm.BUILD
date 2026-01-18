# BUILD file for prebuilt LLVM/Clang toolchain
# Exposes clang, lld, and other tools for cross-compilation

package(default_visibility = ["//visibility:public"])

filegroup(
    name = "all_files",
    srcs = glob(["**/*"]),
)

# Clang compiler
filegroup(
    name = "clang",
    srcs = ["bin/clang"],
)

filegroup(
    name = "clang++",
    srcs = ["bin/clang++"],
)

# LLD linker
filegroup(
    name = "lld",
    srcs = ["bin/ld.lld"],
)

# LLVM tools
filegroup(
    name = "llvm-ar",
    srcs = ["bin/llvm-ar"],
)

filegroup(
    name = "llvm-nm",
    srcs = ["bin/llvm-nm"],
)

filegroup(
    name = "llvm-objcopy",
    srcs = ["bin/llvm-objcopy"],
)

filegroup(
    name = "llvm-objdump",
    srcs = ["bin/llvm-objdump"],
)

filegroup(
    name = "llvm-strip",
    srcs = ["bin/llvm-strip"],
)

filegroup(
    name = "llvm-dwp",
    srcs = ["bin/llvm-dwp"],
)

filegroup(
    name = "llvm-cov",
    srcs = ["bin/llvm-cov"],
)

# All compiler files needed for compilation
filegroup(
    name = "compiler_files",
    srcs = [
        ":clang",
        ":clang++",
    ] + glob([
        "lib/clang/*/include/**",
    ]),
)

# All linker files needed for linking
filegroup(
    name = "linker_files",
    srcs = [
        ":clang",
        ":clang++",
        ":lld",
        ":llvm-ar",
    ] + glob([
        "lib/**/*.a",
        "lib/**/*.so*",
    ]),
)

# All tools
filegroup(
    name = "all_tools",
    srcs = [
        ":clang",
        ":clang++",
        ":lld",
        ":llvm-ar",
        ":llvm-nm",
        ":llvm-objcopy",
        ":llvm-objdump",
        ":llvm-strip",
        ":llvm-dwp",
        ":llvm-cov",
    ],
)
