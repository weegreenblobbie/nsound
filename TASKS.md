Claude and I will work on the following.

Bazel build
-----------

[x] Refactor the code base to swtich to build with bazel.
[x] Upgdrade the C++ compiler standandard the clean up the C++ code as needed.
[x] Build with bazel on windows nativel from a PowerShell window, maybe add a setup-windows.ps1 to install bazel for windows?

Catch2 unit tests
-----------------

[x] Add unit test coverage for 100% the C++ code, along with code coverage reports.
[x] Switch to using std::make_shared<T> and std::make_unique<T> where possible.
[x] Update the class for C++ Core Guidelines C.48 and C.45 so we use class difinition initializers
[x] Switch raw array allocations (new [] T) with std::vector<T> to avoid new and delete
[x] What other C++ best practicies should we incorporate?
[x] Switch to using #pragma once for headers

Switch to using pybind11 and verify plotting
--------------------------------------------

[x] Refactor the code base to replace thw swig python binding with pybind11
[x] Let's working on the c++ linking with the python runtime for matplotlib to make plots
[x] Add unit test coverage for every plot function, using a non-graphcial matploblib backend for image comparison
[x] I can verify with the cepstral_pitch_estimate example

Support audio playback
----------------------

[ ] How can we test audio playback in linux bazel test builds?
[ ] Update the windows bazel build so AudioPlayback works, can it be tested with bazel test too?

Publish to pypi
---------------
[ ] Publish a new nsound pyton package on pypi
