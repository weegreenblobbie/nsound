# Dev container for nsound: C++ (GCC + Clang), Bazel, Catch2 unit tests,
# code coverage, and the Python/pybind11 bindings.
#
# Build:  docker build -t nsound-claude .
# Update Claude Code / tools later with:  docker build --no-cache -t nsound-claude .

# Debian trixie: GCC 14, Clang 19, lcov 2.x.
# Python 3.11, the module builds with Python 3.10 or newer.
FROM python:3.11-slim-trixie

ARG TARGETARCH=amd64

# Non-root user (--dangerously-skip-permissions refuses to run as root)
RUN useradd -m -s /bin/bash ubuntu

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
        # nsound's optional audio backends (same as its GitHub Actions job)
        portaudio19-dev libao-dev \
    && rm -rf /var/lib/apt/lists/*

# Windows bind mounts show up with a different owner, which makes git refuse
# to work ("dubious ownership"). Trust the mounted workspace.
RUN git config --system --add safe.directory /workspace

# Bazelisk, installed as "bazel": it reads .bazelversion and fetches the
# right Bazel automatically.
RUN curl -fsSL -o /usr/local/bin/bazel \
        "https://github.com/bazelbuild/bazelisk/releases/latest/download/bazelisk-linux-${TARGETARCH}" \
    && chmod +x /usr/local/bin/bazel \
    && ln -s /usr/local/bin/bazel /usr/local/bin/bazelisk

# Python tools: pytest, coverage reports, numpy/matplotlib for nsound's
# plotting and Python tests, and pybind11. "pybind11[global]" also installs
# the headers and CMake config into the system prefix so any build can find
# them; "python -m pybind11 --includes" prints the include flags.
# scikit-build-core + build are for packaging the pybind11 module as a wheel.
RUN pip install --no-cache-dir --upgrade pip \
    && pip install --no-cache-dir \
        wheel setuptools build \
        pytest numpy matplotlib \
        gcovr \
        "pybind11[global]" scikit-build-core pybind11-stubgen

USER ubuntu

# Caches (Bazel, Bazelisk, ccache) live here. The launch script mounts a named
# Docker volume on this folder so builds stay fast between runs.
RUN mkdir -p /home/ubuntu/.cache

# Claude Code via the native installer (npm install is deprecated).
ENV PATH="/home/ubuntu/.local/bin:${PATH}"
RUN curl -fsSL https://claude.ai/install.sh | bash \
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