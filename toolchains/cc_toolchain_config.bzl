"""Clang toolchain configuration for cross-compilation.

This toolchain uses Clang as a cross-compiler. Clang can target any
architecture by specifying --target=<triple>.

The toolchain uses prebuilt LLVM binaries downloaded from official releases,
so no system packages are required.

Requirements:
  - For cross-compilation: target sysroot with headers and libraries
"""

load("@bazel_tools//tools/cpp:cc_toolchain_config_lib.bzl",
    "feature",
    "flag_group",
    "flag_set",
    "tool_path",
)
load("@bazel_tools//tools/build_defs/cc:action_names.bzl", "ACTION_NAMES")

# All C++ compile actions
ALL_CPP_COMPILE_ACTIONS = [
    ACTION_NAMES.cpp_compile,
    ACTION_NAMES.cpp_header_parsing,
    ACTION_NAMES.cpp_module_compile,
    ACTION_NAMES.cpp_module_codegen,
]

# All C compile actions
ALL_C_COMPILE_ACTIONS = [
    ACTION_NAMES.c_compile,
]

# All link actions
ALL_LINK_ACTIONS = [
    ACTION_NAMES.cpp_link_executable,
    ACTION_NAMES.cpp_link_dynamic_library,
    ACTION_NAMES.cpp_link_nodeps_dynamic_library,
]

def _impl(ctx):
    target_triple = ctx.attr.target_triple
    target_cpu = ctx.attr.target_cpu
    toolchain_identifier = ctx.attr.toolchain_identifier
    llvm_prefix = ctx.attr.llvm_prefix
    sysroot = ctx.attr.sysroot
    extra_compile_flags = ctx.attr.extra_compile_flags
    extra_link_flags = ctx.attr.extra_link_flags

    # Tool paths using the LLVM prefix
    bin_prefix = llvm_prefix + "/bin/"
    tool_paths = [
        tool_path(name = "gcc", path = bin_prefix + "clang"),
        tool_path(name = "g++", path = bin_prefix + "clang++"),
        tool_path(name = "cpp", path = bin_prefix + "clang-cpp"),
        tool_path(name = "ar", path = bin_prefix + "llvm-ar"),
        tool_path(name = "ld", path = bin_prefix + "ld.lld"),
        tool_path(name = "nm", path = bin_prefix + "llvm-nm"),
        tool_path(name = "objcopy", path = bin_prefix + "llvm-objcopy"),
        tool_path(name = "objdump", path = bin_prefix + "llvm-objdump"),
        tool_path(name = "strip", path = bin_prefix + "llvm-strip"),
        tool_path(name = "dwp", path = bin_prefix + "llvm-dwp"),
        tool_path(name = "llvm-cov", path = bin_prefix + "llvm-cov"),
        tool_path(name = "gcov", path = bin_prefix + "llvm-cov"),
    ]

    # Common compiler flags
    common_compile_flags = [
        "-Wall",
        "-Wno-switch",
        "-fPIC",
        "-fno-common",
        "-fno-omit-frame-pointer",
    ]

    # Target-specific flags
    target_flags = ["--target=" + target_triple] + list(extra_compile_flags)

    # Sysroot flags (if specified)
    sysroot_flags = []
    if sysroot:
        sysroot_flags = ["--sysroot=" + sysroot]

    # C++ standard
    cxx_flags = ["-std=c++20"]

    # Optimization levels
    opt_flags = ["-O2"]
    dbg_flags = ["-g", "-O0"]

    # Features
    features = [
        # Default compile flags
        feature(
            name = "default_compile_flags",
            enabled = True,
            flag_sets = [
                flag_set(
                    actions = ALL_C_COMPILE_ACTIONS + ALL_CPP_COMPILE_ACTIONS,
                    flag_groups = [
                        flag_group(flags = common_compile_flags + target_flags + sysroot_flags),
                    ],
                ),
                flag_set(
                    actions = ALL_CPP_COMPILE_ACTIONS,
                    flag_groups = [
                        flag_group(flags = cxx_flags),
                    ],
                ),
            ],
        ),

        # Default link flags
        feature(
            name = "default_link_flags",
            enabled = True,
            flag_sets = [
                flag_set(
                    actions = ALL_LINK_ACTIONS,
                    flag_groups = [
                        flag_group(flags = [
                            "--target=" + target_triple,
                            "-fuse-ld=lld",
                            "-lpthread",
                            "-lrt",
                            "-lm",
                        ] + list(extra_link_flags) + sysroot_flags),
                    ],
                ),
            ],
        ),

        # Optimization feature
        feature(
            name = "opt",
            flag_sets = [
                flag_set(
                    actions = ALL_C_COMPILE_ACTIONS + ALL_CPP_COMPILE_ACTIONS,
                    flag_groups = [flag_group(flags = opt_flags)],
                ),
            ],
        ),

        # Debug feature
        feature(
            name = "dbg",
            flag_sets = [
                flag_set(
                    actions = ALL_C_COMPILE_ACTIONS + ALL_CPP_COMPILE_ACTIONS,
                    flag_groups = [flag_group(flags = dbg_flags)],
                ),
            ],
        ),

        # Support for PIC
        feature(
            name = "supports_pic",
            enabled = True,
        ),

        # Static linking
        feature(
            name = "static_linking_mode",
            flag_sets = [
                flag_set(
                    actions = ALL_LINK_ACTIONS,
                    flag_groups = [flag_group(flags = ["-static"])],
                ),
            ],
        ),

        # User compile flags (from copts)
        feature(
            name = "user_compile_flags",
            enabled = True,
            flag_sets = [
                flag_set(
                    actions = ALL_C_COMPILE_ACTIONS + ALL_CPP_COMPILE_ACTIONS,
                    flag_groups = [
                        flag_group(
                            flags = ["%{user_compile_flags}"],
                            iterate_over = "user_compile_flags",
                            expand_if_available = "user_compile_flags",
                        ),
                    ],
                ),
            ],
        ),

        # Unfiltered compile flags (reproducible builds)
        feature(
            name = "unfiltered_compile_flags",
            enabled = True,
            flag_sets = [
                flag_set(
                    actions = ALL_C_COMPILE_ACTIONS + ALL_CPP_COMPILE_ACTIONS,
                    flag_groups = [
                        flag_group(
                            flags = [
                                "-no-canonical-prefixes",
                                "-Wno-builtin-macro-redefined",
                                "-D__DATE__=\"redacted\"",
                                "-D__TIMESTAMP__=\"redacted\"",
                                "-D__TIME__=\"redacted\"",
                            ],
                        ),
                    ],
                ),
            ],
        ),
    ]

    # Builtin include directories from the LLVM installation
    cxx_builtin_include_directories = [
        llvm_prefix + "/lib/clang/17/include",
        llvm_prefix + "/include/c++/v1",
        "/usr/include",
        "/usr/local/include",
    ]

    # Add sysroot include directories for cross-compilation
    if sysroot:
        cxx_builtin_include_directories += [
            sysroot + "/usr/include",
            sysroot + "/include",
        ]

    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        features = features,
        toolchain_identifier = toolchain_identifier,
        host_system_name = "x86_64-unknown-linux-gnu",
        target_system_name = target_triple,
        target_cpu = target_cpu,
        target_libc = "glibc",
        compiler = "clang",
        abi_version = "clang",
        abi_libc_version = "glibc",
        tool_paths = tool_paths,
        cxx_builtin_include_directories = cxx_builtin_include_directories,
    )

cc_toolchain_config = rule(
    implementation = _impl,
    attrs = {
        "target_cpu": attr.string(mandatory = True, doc = "Target CPU architecture"),
        "target_triple": attr.string(mandatory = True, doc = "Target triple (e.g., arm-unknown-linux-gnueabihf)"),
        "toolchain_identifier": attr.string(mandatory = True, doc = "Unique toolchain identifier"),
        "llvm_prefix": attr.string(
            default = "external/llvm_toolchain",
            doc = "Path to LLVM installation (relative to execroot)",
        ),
        "sysroot": attr.string(default = "", doc = "Path to target sysroot"),
        "extra_compile_flags": attr.string_list(default = [], doc = "Extra flags for compilation (e.g., -march=mips32)"),
        "extra_link_flags": attr.string_list(default = [], doc = "Extra flags for linking"),
    },
    provides = [CcToolchainConfigInfo],
    doc = """
    Creates a Clang-based C++ toolchain configuration using prebuilt LLVM binaries.

    This rule configures clang for cross-compilation by specifying the target
    triple and optional sysroot. Uses prebuilt LLVM binaries from official
    releases - no system packages required.

    Example:
        cc_toolchain_config(
            name = "clang_arm_config",
            target_cpu = "arm",
            target_triple = "arm-unknown-linux-gnueabihf",
            toolchain_identifier = "clang-arm-linux",
            sysroot = "/usr/arm-linux-gnueabihf",
        )
    """,
)
