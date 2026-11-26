"""Builds the Nsound Python module with setuptools and pybind11, for pip.

pyproject.toml holds the package's metadata.  The module is the C++ library
and the bindings in src/python compiled together, the same sources Bazel
builds into //src/python:Nsound.

    pip install .
"""

import glob
import os
import platform
import re
import sys

from pybind11.setup_helpers import ParallelCompile, Pybind11Extension, build_ext
from setuptools import setup

HERE = os.path.dirname(os.path.abspath(__file__))

# Where the generated Nsound.h goes, before src on the include path.
GENERATED_INCLUDE_DIR = os.path.join("build", "nsound_python_h")

# The portaudio backend needs portaudio, which Bazel builds, so the pip build
# leaves it out.
BACKEND_SOURCES = ["AudioBackendLibportaudio.cc"]


def read_version():
    """Returns (A, B, C, version) from version.bzl, e.g. ("0", "10", "0",
    "0.10.0.dev1")."""
    with open(os.path.join(HERE, "version.bzl")) as fd:
        text = fd.read()

    parts = dict(re.findall(r'^NSOUND_VERSION_([ABC]) = "(\d+)"$', text, re.M))
    suffix = re.search(r'^NSOUND_VERSION = "%s\.%s\.%s([^"]*)"', text, re.M).group(1)

    version = "%s.%s.%s%s" % (parts["A"], parts["B"], parts["C"], suffix)

    return parts["A"], parts["B"], parts["C"], version


def platform_os():
    """Returns Nsound.h's NSOUND_PLATFORM_OS_* name for this platform."""
    if sys.platform == "win32":
        return "NSOUND_PLATFORM_OS_WINDOWS"

    if sys.platform == "darwin":
        return "NSOUND_PLATFORM_OS_MAC"

    return "NSOUND_PLATFORM_OS_LINUX"


def target_this_macos():
    """Targets the macOS being built on, unless MACOSX_DEPLOYMENT_TARGET is set.

    pybind11 would target macOS 10.14, where AudioPlaybackRt spins while it
    waits.
    """
    version = platform.mac_ver()[0].split(".")

    # From macOS 11 on, wheels are tagged by the major version, e.g. 15.0.
    if int(version[0]) >= 11:
        target = version[0] + ".0"
    else:
        target = ".".join(version[:2])

    os.environ.setdefault("MACOSX_DEPLOYMENT_TARGET", target)


def write_nsound_h(version_a, version_b, version_c, version):
    """Writes the Python module's Nsound.h from src/Nsound/Nsound.h.in.

    The settings match the nsound_python_h target in src/Nsound/BUILD.bazel,
    keep the two in step.
    """
    substitutions = {
        "@PACKAGE_NAME@": "Nsound",
        "@PACKAGE_VERSION_A@": version_a,
        "@PACKAGE_VERSION_B@": version_b,
        "@PACKAGE_VERSION_C@": version_c,
        "@PACKAGE_VERSION@": version,
        "@PACKAGE_RELEASE@": "Nsound-" + version,
        "@NSOUND_ENDIAN@": "NSOUND_LITTLE_ENDIAN",
        "@NSOUND_64_32_BIT@": "#define NSOUND_64_BIT 1",
        "@NSOUND_M_PI@": "// M_PI is defined in math.h",
        "@NSOUND_CUDA@": "#undef NSOUND_CUDA // disabled",
        "@NSOUND_BOOLEAN@": "bool",
        "@NSOUND_INT_8@": "signed char",
        "@NSOUND_INT_16@": "signed short",
        "@NSOUND_INT_32@": "signed int",
        "@NSOUND_INT_64@": "signed long long",
        "@NSOUND_FLOAT_32@": "float",
        "@NSOUND_FLOAT_64@": "double",
        "@NSOUND_U_INT_8@": "unsigned char",
        "@NSOUND_U_INT_16@": "unsigned short",
        "@NSOUND_U_INT_32@": "unsigned int",
        "@NSOUND_U_INT_64@": "unsigned long long",
        "@NSOUND_RAW_INT_64@": "signed long long",
        "@NSOUND_RAW_U_INT_64@": "unsigned long long",
        "@NSOUND_RAW_FLOAT_64@": "double",
        "@NSOUND_PLATFORM_OS@": platform_os(),
        "@NSOUND_LIBPORTAUDIO@": "#undef NSOUND_LIBPORTAUDIO // disabled",
        "@NSOUND_OPENMP@": "#undef NSOUND_OPENMP // disabled",
        "@NSOUND_IN_PYTHON_MODULE@": "#define NSOUND_IN_PYTHON_MODULE 1",
        "@NSOUND_C_PYLAB@": "#define NSOUND_C_PYLAB 1",
    }

    with open(os.path.join(HERE, "src", "Nsound", "Nsound.h.in")) as fd:
        text = fd.read()

    for key, value in substitutions.items():
        text = text.replace(key, value)

    out_dir = os.path.join(HERE, GENERATED_INCLUDE_DIR, "Nsound")
    os.makedirs(out_dir, exist_ok=True)

    with open(os.path.join(out_dir, "Nsound.h"), "w") as fd:
        fd.write(text)


def sources():
    """Returns the library's and the bindings' C++ sources."""
    library = sorted(
        path
        for pattern in ("src/Nsound/*.cc", "src/Nsound/*.cpp")
        for path in glob.glob(pattern)
        if os.path.basename(path) not in BACKEND_SOURCES
    )

    return library + sorted(glob.glob("src/python/*.cc"))


def main():
    """Generates Nsound.h, then builds the package."""
    os.chdir(HERE)

    version_a, version_b, version_c, version = read_version()

    write_nsound_h(version_a, version_b, version_c, version)

    # Compile on every core, NSOUND_BUILD_JOBS sets the number.
    ParallelCompile("NSOUND_BUILD_JOBS", default=0).install()

    if sys.platform == "win32":
        extra_compile_args = ["/D_CRT_SECURE_NO_WARNINGS"]
    else:
        # The same code generation flags as Bazel's.
        extra_compile_args = ["-fno-strict-aliasing", "-fwrapv"]

    # Before Pybind11Extension, which reads the target.
    if sys.platform == "darwin":
        target_this_macos()

    module = Pybind11Extension(
        "Nsound._Nsound",
        sources(),
        cxx_std=20,
        include_dirs=[GENERATED_INCLUDE_DIR, "src"],
        extra_compile_args=extra_compile_args,
    )

    setup(
        version=version,
        packages=["Nsound"],
        package_dir={"Nsound": "src/python/Nsound"},
        ext_modules=[module],
        cmdclass={"build_ext": build_ext},
    )


main()
