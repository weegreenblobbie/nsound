# Dev container for nsound: C++ (GCC + Clang), Bazel, Catch2 unit tests,
# code coverage, and the Python/pybind11 bindings.
#
# Build:  docker build -t nsound-claude .
# Update Claude Code / tools later with:  docker build --no-cache -t nsound-claude .
#
# Every download is checked against a SHA-256 pinned here or in
# docker_requirements_lock.txt, and the build prints each hash it checked.
# When one doesn't match the build stops and prints the hash it got: check
# the new download is the one you expect before pinning its hash.

# Debian trixie: GCC 14, Clang 19, lcov 2.x.
# Python 3.11, the module builds with Python 3.10 or newer.
# The image by its digest, which Docker checks when it pulls it.
FROM python:3.11-slim-trixie@sha256:07d4824036af339fa1b6ef769cc99d9c10a141769538b52b3b597c84f7a7b535

ARG TARGETARCH=amd64

# Non-root user (--dangerously-skip-permissions refuses to run as root)
RUN useradd -m -s /bin/bash ubuntu

# apt checks the packages against Debian's signed package lists itself.
RUN apt-get update && apt-get install -y --no-install-recommends \
        # basics
        ca-certificates curl git patch bash less procps file \
        zip unzip xz-utils \
        # compilers and build tools
        build-essential g++ gdb \
        clang clang-format clang-tidy clangd lld llvm libomp-dev \
        # clang's sanitizer runtimes (libclang_rt.asan, ubsan, ...)
        libclang-rt-19-dev \
        ccache pkg-config cmake ninja-build \
        # coverage (gcov ships with gcc; llvm-cov/llvm-profdata with llvm)
        lcov \
        # memory and undefined-behaviour checking
        valgrind \
    && rm -rf /var/lib/apt/lists/*

# Windows bind mounts show up with a different owner, which makes git refuse
# to work ("dubious ownership"). Trust the mounted workspace.
RUN git config --system --add safe.directory /workspace

# Bazelisk, installed as "bazel": it reads .bazelversion and fetches the
# right Bazel automatically.  The hashes are the release's .sha256 files.
ARG BAZELISK_VERSION=v1.30.0
ARG BAZELISK_SHA256_amd64=a248ae2b2b62b5baebb748f6fba7203bff1a27e5d01b55399a8406744cb539e8
ARG BAZELISK_SHA256_arm64=b2bcc040c213b44586b121bda6d983c35df121b917c3a7a66fb16f2fa0df1a4f
RUN case "${TARGETARCH}" in \
        amd64) expected="${BAZELISK_SHA256_amd64}" ;; \
        arm64) expected="${BAZELISK_SHA256_arm64}" ;; \
        *) echo "No Bazelisk SHA-256 pinned for ${TARGETARCH}" >&2; exit 1 ;; \
    esac \
    && curl -fsSL -o /tmp/bazelisk \
        "https://github.com/bazelbuild/bazelisk/releases/download/${BAZELISK_VERSION}/bazelisk-linux-${TARGETARCH}" \
    && actual="$(sha256sum /tmp/bazelisk | cut -d' ' -f1)" \
    && if [ "${actual}" != "${expected}" ]; then \
        echo "Bazelisk ${BAZELISK_VERSION}: SHA-256 ${actual}, expected ${expected}" >&2; exit 1; \
    fi \
    && echo "Bazelisk ${BAZELISK_VERSION} linux-${TARGETARCH}: SHA-256 ${actual} passed" \
    && install -m 755 /tmp/bazelisk /usr/local/bin/bazel \
    && rm /tmp/bazelisk \
    && ln -s /usr/local/bin/bazel /usr/local/bin/bazelisk

# Python tools: pytest, coverage reports, numpy/matplotlib for nsound's
# plotting and Python tests, and pybind11. "pybind11[global]" also installs
# the headers and CMake config into the system prefix so any build can find
# them; "python -m pybind11 --includes" prints the include flags.
# scikit-build-core + build are for packaging the pybind11 module as a wheel.
# docker_requirements.in lists them, the lock pins every package and its
# hashes, which --require-hashes checks.
COPY docker_requirements_lock.txt /tmp/docker_requirements_lock.txt
RUN pip install --no-cache-dir --require-hashes -r /tmp/docker_requirements_lock.txt \
    && echo "pip: every package passed its SHA-256 in docker_requirements_lock.txt" \
    && rm /tmp/docker_requirements_lock.txt

USER ubuntu

# Caches (Bazel, Bazelisk, ccache) live here. The launch script mounts a named
# Docker volume on this folder so builds stay fast between runs.
RUN mkdir -p /home/ubuntu/.cache

# Claude Code via the native installer (npm install is deprecated).  The
# installer is checked against the hash pinned here; it checks the Claude
# binary it downloads against the release manifest's SHA-256 itself.
ENV PATH="/home/ubuntu/.local/bin:${PATH}"
ARG CLAUDE_INSTALL_SHA256=2c5506d110560bf618bbe88f678f2c2a0047c86a9c6585fc84976120b1de0b44
RUN curl -fsSL -o /tmp/claude-install.sh https://claude.ai/install.sh \
    && actual="$(sha256sum /tmp/claude-install.sh | cut -d' ' -f1)" \
    && if [ "${actual}" != "${CLAUDE_INSTALL_SHA256}" ]; then \
        echo "claude.ai/install.sh: SHA-256 ${actual}, expected ${CLAUDE_INSTALL_SHA256}" >&2; exit 1; \
    fi \
    && echo "claude.ai/install.sh: SHA-256 ${actual} passed" \
    && bash /tmp/claude-install.sh \
    && rm /tmp/claude-install.sh \
    && claude --version

# DISABLE_AUTOUPDATER: the container is thrown away on exit, so auto-updates
#   would just re-download every run. Rebuild the image to update instead.
# MPLBACKEND: matplotlib draws without a display.
# CCACHE_DIR: keep ccache in the persistent cache volume.
ENV CLAUDE_CONFIG_DIR="/home/ubuntu/.claude" \
    DISABLE_AUTOUPDATER=1 \
    MPLBACKEND=Agg \
    CCACHE_DIR=/home/ubuntu/.cache/ccache \
    CC=gcc \
    CXX=g++

# Sanity check that the toolchain is all there
RUN gcc --version | head -1 \
    && clang --version | head -1 \
    && test -x /usr/local/bin/bazel \
    && python -m pybind11 --includes \
    && lcov --version \
    && gcovr --version | head -1

WORKDIR /workspace

# ENTRYPOINT + empty CMD means extra "docker run" arguments are appended to
# claude, e.g.  docker run ... nsound-claude --resume <uuid>
ENTRYPOINT ["claude", "--dangerously-skip-permissions"]
CMD []