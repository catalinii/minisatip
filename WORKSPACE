workspace(name = "minisatip")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Rules for C++
http_archive(
    name = "rules_cc",
    sha256 = "2037875b9a4456dce4a79d112a8ae885bbc4aad968e6587dca6e64f3a0900cdc",
    strip_prefix = "rules_cc-0.0.9",
    urls = ["https://github.com/bazelbuild/rules_cc/releases/download/0.0.9/rules_cc-0.0.9.tar.gz"],
)

# Rules for building foreign (autoconf/cmake) projects
http_archive(
    name = "rules_foreign_cc",
    sha256 = "476303bd0f1b04cc311fc258f1708a5f6ef82d3091e53fd1977fa20383425a6a",
    strip_prefix = "rules_foreign_cc-0.10.1",
    url = "https://github.com/bazelbuild/rules_foreign_cc/releases/download/0.10.1/rules_foreign_cc-0.10.1.tar.gz",
)

load("@rules_foreign_cc//foreign_cc:repositories.bzl", "rules_foreign_cc_dependencies")

rules_foreign_cc_dependencies()

# libdvbcsa source
http_archive(
    name = "dvbcsa",
    build_file = "@//:third_party/dvbcsa.BUILD",
    sha256 = "4db78af5cdb2641dfb1136fe3531960a477c9e3e3b6ba19a2754d046af3f456d",
    strip_prefix = "libdvbcsa-1.1.0",
    urls = ["https://download.videolan.org/pub/videolan/libdvbcsa/1.1.0/libdvbcsa-1.1.0.tar.gz"],
)

# Cross-compilation toolchains are defined in //toolchains
# Use them with --config=clang-arm, --config=clang-arm64, etc.
# See .bazelrc for configuration options.

# LLVM/Clang prebuilt binaries for cross-compilation
# Downloaded from official LLVM releases - no system packages required
# Separate archives for x86_64 and aarch64 hosts

# LLVM for x86_64 hosts (Intel/AMD)
http_archive(
    name = "llvm_toolchain_x86_64",
    build_file = "@//:third_party/llvm.BUILD",
    sha256 = "884ee67d647d77e58740c1e645649e29ae9e8a6f7571a8a5c9e9918b1c58a9fc",
    strip_prefix = "clang+llvm-17.0.6-x86_64-linux-gnu-ubuntu-22.04",
    urls = ["https://github.com/llvm/llvm-project/releases/download/llvmorg-17.0.6/clang+llvm-17.0.6-x86_64-linux-gnu-ubuntu-22.04.tar.xz"],
)

# LLVM for aarch64 hosts (ARM64 servers, Apple Silicon via Rosetta, etc.)
http_archive(
    name = "llvm_toolchain_aarch64",
    build_file = "@//:third_party/llvm.BUILD",
    sha256 = "6dd62762285326f223f40b8e4f2864b5c372de3f7de0731cb7cd55ca5287b75a",
    strip_prefix = "clang+llvm-17.0.6-aarch64-linux-gnu",
    urls = ["https://github.com/llvm/llvm-project/releases/download/llvmorg-17.0.6/clang+llvm-17.0.6-aarch64-linux-gnu.tar.xz"],
)
