//-----------------------------------------------------------------------------
//
//  Lets the examples work under `bazel run`, see run_from_examples_dir().
//
//-----------------------------------------------------------------------------
#pragma once

#include <cstdlib>
#include <filesystem>

//! Changes into <repo>/src/examples when started by `bazel run`.
//
//  The examples read and write .wav files in the current folder, but
//  `bazel run` starts them in a read-only runfiles folder.  It also sets
//  BUILD_WORKSPACE_DIRECTORY to the repo root, so move to src/examples
//  there.  Outside `bazel run` the variable is not set and nothing changes.
inline
void
run_from_examples_dir()
{
    const char * workspace = std::getenv("BUILD_WORKSPACE_DIRECTORY");

    if(workspace != nullptr)
    {
        std::filesystem::current_path(
            std::filesystem::path(workspace) / "src" / "examples");
    }
}

