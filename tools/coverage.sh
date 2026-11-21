#!/usr/bin/env bash
# Runs the C++ unit tests under coverage and writes an HTML report.
#
# Usage: tools/coverage.sh [output_dir] [extra bazel args...]
#
# The lcov report is bazel-out/_coverage/_coverage_report.dat, the HTML
# report goes to output_dir (default: coverage_html).

set -euo pipefail

cd "$(dirname "$0")/.."

output_dir="${1:-coverage_html}"
shift || true

bazel coverage --combined_report=lcov "$@" //src/test:unit_tests

report="$(bazel info output_path)/_coverage/_coverage_report.dat"

genhtml "${report}" --output-directory "${output_dir}"

echo "HTML coverage report: ${output_dir}/index.html"
