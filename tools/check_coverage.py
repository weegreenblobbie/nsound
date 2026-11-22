#!/usr/bin/env python3
"""Fails when the src/Nsound line coverage in an lcov report drops below the minimum.

Usage: tools/check_coverage.py <lcov report>

tools/coverage.sh runs it on the report from `bazel coverage`.
"""

import collections
import sys

# The coverage reached so far, rounded down to 0.1%. Raise it as tests are
# added so it can only go up.
MINIMUM_PERCENT = 96.0

# The library code that is measured.
MEASURED_PREFIX = "src/Nsound/"

# Code that can't run in CI because it needs a sound card or an optional
# library, matched by path prefix.
EXCLUDED_PREFIXES = (
    "src/Nsound/AudioBackendLibao.",        # libao, --//bazel:libao
    "src/Nsound/AudioBackendLibportaudio.", # portaudio, --//bazel:portaudio
    "src/Nsound/AudioPlayback.",            # plays through an audio backend
    "src/Nsound/AudioPlaybackRt.",          # real time playback, portaudio
)


def read_line_hits(report_path: str) -> dict[str, dict[int, int]]:
    """Returns {source file: {line number: hit count}} from an lcov report.

    A file can appear once per test binary, its lines are counted as hit if
    any test hit them.
    """
    hits: dict[str, dict[int, int]] = collections.defaultdict(dict)
    source = None

    with open(report_path, encoding="utf-8") as report:
        for line in report:
            line = line.strip()
            if line.startswith("SF:"):
                source = line[3:]
            elif line.startswith("DA:") and source is not None:
                number, count = line[3:].split(",")[:2]
                lines = hits[source]
                lines[int(number)] = max(lines.get(int(number), 0), int(count))
            elif line == "end_of_record":
                source = None

    return hits


def is_measured(source: str) -> bool:
    """True for src/Nsound files that are not excluded."""
    return source.startswith(MEASURED_PREFIX) and not source.startswith(EXCLUDED_PREFIXES)


def main(argv: list[str]) -> int:
    """Prints the measured coverage and returns 1 when it is below the minimum."""
    if len(argv) != 2:
        print(__doc__.strip(), file=sys.stderr)
        return 2

    hits = read_line_hits(argv[1])

    measured = [lines for source, lines in hits.items() if is_measured(source)]
    total = sum(len(lines) for lines in measured)
    hit = sum(1 for lines in measured for count in lines.values() if count > 0)

    if total == 0:
        print(f"No {MEASURED_PREFIX} lines in {argv[1]}", file=sys.stderr)
        return 1

    percent = 100.0 * hit / total

    print(f"{MEASURED_PREFIX} line coverage: {percent:.1f}% ({hit} of {total} lines), "
          f"minimum {MINIMUM_PERCENT:.1f}%")

    if percent < MINIMUM_PERCENT:
        print("Coverage dropped below the minimum in tools/check_coverage.py",
              file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
