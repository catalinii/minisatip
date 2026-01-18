load("@rules_foreign_cc//foreign_cc:defs.bzl", "configure_make")

filegroup(
    name = "all_srcs",
    srcs = glob(
        ["**"],
        exclude = ["*.bazel"],
    ),
)

configure_make(
    name = "dvbcsa",
    args = ["-j4"],
    configure_in_place = True,
    configure_options = [
        "--enable-static",
        "--disable-shared",
    ],
    lib_name = "dvbcsa",
    lib_source = ":all_srcs",
    out_include_dir = "include",
    out_static_libs = ["libdvbcsa.a"],
    visibility = ["//visibility:public"],
)
