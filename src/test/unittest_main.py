"""Runs the unittest test cases of the modules named on the command line.

The Python test files have no main(), Bazel's py_test runs them with this.
"""

import os
import sys
import unittest


def use_real_package_paths():
    """Puts Bazel's real copies of the packages on sys.path, not the runfiles.

    rules_python puts the packages' runfiles folders on sys.path.  Their real
    copies, in Bazel's external folder, have paths about 100 characters
    shorter, and the Windows DLL loader can't find a DLL's DLLs past 260
    characters: Pillow's _imaging, which matplotlib imports, fails to load
    from the runfiles.  The runfiles manifest names each runfile's real file.
    The main repository's folders stay, their files come from the source
    tree and bazel-out, not one folder.
    """
    runfiles = os.environ.get("RUNFILES_DIR")

    if not runfiles:
        return

    manifest = os.environ.get("RUNFILES_MANIFEST_FILE") or os.path.join(runfiles, "MANIFEST")

    if not os.path.isfile(manifest):
        return

    real_files = []

    with open(manifest, encoding="utf-8") as fd:
        for line in fd:
            runfile, _, real = line.rstrip("\n").partition(" ")
            if real:
                real_files.append((runfile, real))

    for i, path in enumerate(sys.path):
        try:
            folder = os.path.relpath(path, runfiles).replace(os.sep, "/")
        except ValueError:
            # A path on another Windows drive.
            continue

        if folder.startswith("..") or folder == "." or folder.split("/")[0] == "_main":
            continue

        # A runfile in the folder: its real path ends with the same file
        # path, the rest is the real folder.
        for runfile, real in real_files:
            if runfile.startswith(folder + "/") and real.endswith(runfile[len(folder):]):
                real_folder = os.path.normpath(real[:-len(runfile[len(folder):])])

                if os.path.isdir(real_folder):
                    sys.path[i] = real_folder

                break


if __name__ == "__main__":
    use_real_package_paths()
    unittest.main(module=None)
