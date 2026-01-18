load("@rules_foreign_cc//foreign_cc:defs.bzl", "configure_make")

filegroup(
    name = "all_srcs",
    srcs = glob(
        ["**"],
        exclude = ["*.bazel"],
    ),
)

configure_make(
    name = "openssl",
    args = ["-j4"],
    configure_command = "config",
    configure_in_place = True,
    configure_options = [
        "no-shared",
        "no-tests",
        "no-legacy",
    ],
    env = {
        "AR": "",
    },
    lib_name = "openssl",
    lib_source = ":all_srcs",
    out_include_dir = "include",
    out_static_libs = [
        "libssl.a",
        "libcrypto.a",
    ],
    targets = [
        "build_libs",
        "install_dev",
    ],
    visibility = ["//visibility:public"],
)
