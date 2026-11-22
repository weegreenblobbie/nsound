//-----------------------------------------------------------------------------
//
//  main() for the Catch2 unit tests.
//
//-----------------------------------------------------------------------------

#include <catch2/catch_session.hpp>

#include <cstdlib>
#include <filesystem>

//! LeakSanitizer's suppressions, under --config=asan: the Python that Plotter
//! starts runs until the program ends, it and its packages keep their memory.
extern "C"
const char *
__lsan_default_suppressions()
{
    return
        "leak:libpython\n"
        "leak:site-packages\n";
}

int main(int argc, char ** argv)
{
    // Under `bazel test` the working directory is the read-only runfiles tree,
    // so run from TEST_TMPDIR with a copy of the runfiles gold files.  The
    // tests read gold/*.wav and some write .wav files to the current folder.
    const char * test_srcdir = std::getenv("TEST_SRCDIR");
    const char * test_tmpdir = std::getenv("TEST_TMPDIR");

    if(test_srcdir != nullptr && test_tmpdir != nullptr)
    {
        namespace fs = std::filesystem;

        fs::path gold = fs::path(test_srcdir) / "_main" / "src" / "test" / "gold";

        fs::copy(gold, fs::path(test_tmpdir) / "gold", fs::copy_options::recursive);
        fs::current_path(test_tmpdir);
    }

    return Catch::Session().run(argc, argv);
}
