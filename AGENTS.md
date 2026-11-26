# Nsound

This file provides guidance to AI agents while working in this repo.  These rules apply to all of nsound, nested `AGENTS.md` 
add or override rules for their subtree.

# Overview

Nsound is a C++20 library and Python module for audio synthesis featuring dynamic digital filters.  It builds with Bazel on Linux, macOS and Windows.  `README.rst` describes the build, the targets and the feature flags.

## Directory Structure:
- `src/Nsound/` - The C++ library.
- `src/python/` - The pybind11 bindings of the Python module.
- `src/test/` - Catch2 C++ tests (`<Class>_test.cc`), Python `unittest` tests (`test_*.py`), manual test programs (`test_*.cc`) and the gold files in `gold/`.
- `src/examples/` - C++ and Python example programs.
- `src/bin/` - Command line programs.
- `bazel/` - Build flags, Bazel rules and dependency patches.
- `tools/` - Coverage and `pip install` test scripts.
- `docs/` - The user guide and reference documentation.
- `claude-cfg/`, `agy-cfg/` - The agents' config and logins, not part of the committed code tree, please ignore.

# Key Concepts:
- `bazel build //...` builds everything and `bazel test //...` runs the C++ and Python unit tests.  `README.rst` lists the clang, sanitizer and coverage configs.
- Warnings in files under `src/` are errors.
- `setup.py` builds the Python module for `pip`, separately from Bazel.  A build change must work in both, `tools/test_pip_install.py` tests the `pip` one.
- `src/Nsound/Nsound.h` is generated from `Nsound.h.in` by Bazel and by `setup.py`, edit the template.  The version is in `version.bzl`.
- The Linux container and Windows share the source tree.  Changes must work with gcc, clang and MSVC.
- `Dockerfile` and `start-agent.ps1` make and start the agents' container.
- `AGENTS.md` holds the immutable rules, `README.md` and `README.rst` may contain tool help or code local documentation.
- Prioritize in order: correctness, readaility, performance.
- Python dependencies are maintained in these files: `requirements.in` (Bazel), `docker_requirements.in` (the container) and `pyproject.toml` (the package).  Each `.in` file has a `*_lock.txt` file made by the `uv` command at the lock's top.

# Patterns to avoid
- Do not write local python script for editing files, use standard patch files instead.
- Do not alter surrounding code, formatting, variable names, or whitespace unless required to do so.

# Agent behavior and code generation 

## 1. Docmunation discovery

Before changing any files in a directory, check for and read any local AGENTS.md and README.md fiels in the directory path.

Discovery process:
1. Ideinfy the directory you are working in, i.e. `src/test/`, `src/Nsound/`, etc.
2. Check if documentation files exist at that level:
  * `test -f <directory>/AGENTS.md` - Rules and constraints for this subtree
  * `test -f <directory>/README.md` - Context, architecture, and guidance for this subtree
3. Read both files completely before making changes. 
  * `AGENTS.md`: Apply both root rules AND subtree rules (subtree takes precedence on conflicts)
  * `README.md`: Use for understand the subsystem, its purpose, structure, and conventions
4. If working in nested subdirectories, check each level for documentation.

## 2. Scope

The scope is what the user names: a function, a file, a set of lines.  when the user names a unit, the whole unit is in scope.  Touch only the files within scope.

Outside the scope:
- Do not "improve" adjacent code, comments, for formatting.
- Do not refactor things that are not broken.
- Match existing style, even if you would do it differently.
- If you find urelated dead code, tell the user, do not delete it.

Inside the scope:
- Apply the rqeust fully.  Do not stop at the first few changes.
- When you apply a coding standard, check every lin in scope against that standard.
- "Few edits" is not the goal. "No edits outside scope" is the goal.

When your changes create orphans:
- Remove iports/variables/functions that your changes made unused.
- Don't remove pre-exising dead code unless asked

The test has two parts:
- Every changed line traces to the user's request.
- No line inside scope is elft unfixed when the request covers it.

Only implement the absolute minimum changeset (code) to solve the underlying problem.  Do not speculate.

## 3. Pre-change checklist

Before any file modification (edit, write, create), run through this mandatory checklist:

Once per session or when entring a new subtree:
* (1) Local documentation: Check for and read any subtree AGENTS.md and README.md

Before every batch of changes:
* (2) Scope statement: Explicityly state what is in scope and what is out of scope.
* (3) Rule verification: Verify your planned approcah does not violate any applicale rules.

If any item fails: If local documentation conflicts with the approach, or scope is unclear, STOP and ask the user.

Not a formality: This is a gate, not a ritual.  If you cannot complete the checklist, do not proceed with changes.

## 4. goal driven execution

Tramsform tasks into verifiale gaols:
- Add validation -> write test for invalid inputs, then make them pass.
- Fix the bug -> write a test that reproduces it, then make it pass.
- Refactor X -> Ensure tests pass before and after.

For multi-step tasks, state a brief plan:
```
1. [Step] -> verify: [check]
2. [Step] -> verify: [check]
3. [Step] -> verify: [check]
```

## 5. Least surprise

The read shoudl know what the name does from teh name or the frist few lines.  Do not add "does x except when y and z" paths.  Do not add hidden coupling with other parts of the system.

## 6. Documentation

write comments and docs that explain wht the remaining code exisdts.  Do not write a changelog of deletions.  Do not describe features, paths, or types that are not in the code.

## 7. Rule of three

Do no design for reuse, genericism, or deduplication until you have three real uses. Divergent uses may stay divergent.  Each implementation should do one thing with a clear set of preconditions.

## 8. Data layout first

Big-O compares how one algorithm scales.  It does not choose between differnt algorithms. Constantd factors often dominate.  Fix layouts and interfaces before you micro-optimze a function.

## 9. Naming principles

- Name specificity shoudl match scope (except for loop indicies like `i`, `j`, `k`, etc)
- No cute names (pop culture, mythology), acronyms, or >4 syllable names
- If meaning changes, change name everywhere
- Singular for loop elements: `for bar in bars` not `for foo in bars`

## 10. Prduction order

Working code is not enough. Correctness comes first.  Maintainability comes second. Performance comes third, and it comes from data layout and intefaces, not form local hotspot edits.

## 12. Rules aplies to all code

These rules apply to all the code you write, and all the code that you are asked to review.

## 13. Obvious

All names and vehavior should be obvious from reading.

## 14. Edge cases

Do not write surprising behavior or extra edge cases.

## 15. Choices

If multiple interpretations exist, present them - don't pick silently.

## 16. Clarity

If something is unclear, STOP.  Name what's confusing. Ask. State any assumptions you maybe have explicity.

## 17. Goals

Every turn should be goal-drivin.  Create success criteria and loop until you have verified your changes.

## 18. Immutability

Some functions have been marked IMMUTABLE in their docstrings or header comments, these must never be changed. If you want to change them, STOP.  Ask the user to make the changes for you if they agree to make them.

## 19. C++ design

* **Headers:** Use `#pragma once`.
* **Ownership:** Create owned objects with `std::make_unique<T>` or `std::make_shared<T>` where possible, and use `std::vector<T>` instead of `new[]` and `delete[]`.
* **Member Initialization:** Follow C++ Core Guidelines C.45 and C.48, give members their constant initial values in the class definition.
* **Untrusted Input:** Validate what is read from files and byte strings before using it.

## 21. Unit tests

To ensure robust test coverage and catch edge cases, all unit tests must adhere to the following rules:

1. **Distinct Dimensions:** Any test utilizing multi-dimensional data must use different sizes and contents per dimension (e.g., 2 channels of 13 samples with different values in each channel). This prevents axis-handling errors that remain hidden when dimensions or channels are identical.

2. **Value Flow Verification:** Tests must prove that values actually flow through the logic. Do not solely initialize parameters with `0` or `0.0` and assert `0` or `0.0`. Always include test cases with non-zero, distinct values to positively confirm the code correctly applies transformations, assignments, and state updates.

3. **Single Source of Truth (SSOT) for Diagnostics & Testing:** Never re-implement, duplicate, or independently re-calculate core mathematical transforms, masks, or intermediate states inside test scripts or diagnostic plotting functions. Diagnostic and test scripts must consume the exact arrays and values returned directly by the core engine functions.

4. **Headless:** Tests run without a sound card or a display.  Plot tests use matplotlib's Agg backend, playback tests capture the audio into a file.

5. **Coverage:** `tools/coverage.sh` fails if the `src/Nsound` line coverage drops below `MINIMUM_PERCENT` in `tools/check_coverage.py`.  Raise it as tests are added, never lower it.

6. **Python Defines Behavior:** Before changing what a C++ method does, search `src/test/*.py` and `src/examples/*.py` for its uses.

## 22. Be Frugle

- Do no show code diffs in the terminal that waste tokens and context.  The user can use `git diff` to see changes. You only need to summarise what files and or functions were edited.
- Do not write logging or debug print statements that spam the console with 10s, 100s or 1000s of lines, that also wastes context and tokens.