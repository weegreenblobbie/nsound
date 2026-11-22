"""Tests installing the Nsound Python module into a new venv.

Creates a venv in a temporary folder, pip installs this checkout into it
with numpy and matplotlib, checks Nsound imports from the venv, then runs
the Python tests (src/test/test_*.py) with the venv's Python.  The venv is
deleted afterwards, unless --keep is given.

    python tools/test_pip_install.py        # Linux, macOS
    py tools\\test_pip_install.py            # Windows

It runs with the Python that runs it, 3.10 or newer, and needs a C++20
compiler and the network, for pip.
"""

import argparse
import os
import shutil
import subprocess
import sys
import tempfile
import venv

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def run(command, **kwargs):
    """Prints and runs command, raises CalledProcessError when it fails."""
    print("+ " + " ".join(command), flush=True)
    subprocess.run(command, check=True, **kwargs)


def venv_python(venv_dir):
    """Returns the path to the venv's python executable."""
    if sys.platform == "win32":
        return os.path.join(venv_dir, "Scripts", "python.exe")

    return os.path.join(venv_dir, "bin", "python")


def main():
    """Creates the venv, installs Nsound, runs the tests."""
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument(
        "--keep", action="store_true", help="keep the venv and print where it is")
    args = parser.parse_args()

    venv_dir = tempfile.mkdtemp(prefix="nsound_venv_")

    try:
        print(f"Creating a venv in {venv_dir}", flush=True)
        venv.create(venv_dir, with_pip=True)
        python = venv_python(venv_dir)

        run([python, "-m", "pip", "install", REPO, "numpy", "matplotlib"])

        # From the venv folder, so the import can't find the source tree.
        check_import = (
            "import os, sys, Nsound\n"
            "print('Nsound', Nsound.__version__, 'from', Nsound.__file__)\n"
            "if not os.path.abspath(Nsound.__file__).startswith(os.path.abspath(sys.prefix)):\n"
            "    sys.exit('Nsound was not imported from the venv')\n"
        )
        run([python, "-c", check_import], cwd=venv_dir)

        # The tests use the Agg backend, they open no windows.
        env = dict(os.environ, MPLBACKEND="Agg")
        run(
            [python, "-m", "unittest", "discover", "-s", os.path.join(REPO, "src", "test"), "-t", REPO],
            cwd=venv_dir,
            env=env)

        print("The venv install works.", flush=True)

    finally:
        if args.keep:
            print(f"Kept the venv: {venv_dir}")
        else:
            shutil.rmtree(venv_dir, ignore_errors=True)


if __name__ == "__main__":
    try:
        main()
    except subprocess.CalledProcessError as error:
        sys.exit(f"Failed: {' '.join(error.cmd)} returned {error.returncode}")
