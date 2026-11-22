<#
.SYNOPSIS
Windows stand-in for tools/coverage.sh: says that coverage isn't supported.

.DESCRIPTION
Bazel collects C++ coverage only with gcc or clang on Linux and macOS, not
with MSVC on Windows, so this script prints where to run it instead and
exits with 1.
#>

Write-Host @"
C++ coverage isn't supported natively on Windows: Bazel collects it only with
gcc or clang on Linux and macOS, not with MSVC.

Run tools/coverage.sh on Linux or macOS instead, for example in the
nsound-claude dev container. The "linux / bazel / coverage" GitHub check
runs it on every pull request.
"@ -ForegroundColor Yellow

exit 1
