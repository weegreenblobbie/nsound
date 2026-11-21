Claude and I will work on the following.

Bazel build
-----------

[x] Refactor the code base to swtich to build with bazel.
[x] Upgdrade the C++ compiler standandard the clean up the C++ code as needed.
[x] Build with bazel on windows nativel from a PowerShell window, maybe add a setup-windows.ps1 to install bazel for windows?

Catch2 unit tests
-----------------

[ ] Add unit test coverage for 100% the C++ code, along with code coverage reports.
[ ] Switch to using std::make_shared<T> and std::make_unique<T> where possible.

Switch to using pybind11
------------------------

[ ] Refactor the code base to replace thw swig python binding with pybind11
[ ] Publish a new nsound pyton package on pypi

Modern C++ improvements
-----------------------

[ ] Switch to using #pragma once for headers

Support audio playback
----------------------

[ ] update the windows bazel build so AudioPlayback works