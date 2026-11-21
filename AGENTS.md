# NSS - Nicks Star Stacker

This file provides guidance to AI agents while working in this repo.  These rules apply to all of nss, nested `AGENTS.md` 
add or override rules for their subtree.

# Overview

`nss` is a python3 based reop.  The root of the repo should be in the default PYTHONPATH so `import nss` works.

## Directory Structure:
- `ai-venv/` - A temporary, headless python3 virtual enviroment for use by ai agents, not part of the committed code tree.  `pytest` must be executed using this venv.  If it is missing, create it with `python -m venv ai-venv` (never use `venv/`) and install `requirements.txt` into it; call its binaries directly (`ai-venv/bin/python`).  Qt tests run headless with `QT_QPA_PLATFORM=offscreen`.
- `nss/` - The main production code comprising tools and utilies for manipulation 16-bit TIFF image files with `numpy`.
- `nss/color_gui/` - The directory of PyQt6 widgets to build out the UI for a color grading application.
- `tests/` - A collection of `pytest` files and data, though you may find `*_test.py` next to the main files in the `nss/` subtree.
- `venv/` - A temporary python3 virtual enviroment that runs nativaly outside the AI agent sandbox, please ignore.

# Key Concepts:
- `nss/tiff_file.py` contains a `TiffFile` class for reading and writing 16-bit TIFF images; it keeps the stored pixel values (no min/max stretch) and the embedded ICC profile, please use it for all image IO.
- `nss.py` is a main entry point for moon image alignment tools.
- `color.py` is the main entry point to starting up the color grading application.
- `pytest` is the tool used for running unit tests.
- `AGENTS.md` holds the immutable rules, `README.md` may contain tool help or code local documentation.
- Prioritize in order: correctness, readaility, performance.
- `requirements.txt` is used for creating python3 virtual enviroments, python dependences must be maintained in this file and shared between AI agents and users of this repo.

# Patterns to avoid
- Do not write local python script for editing files, use standard patch files instead.
- Do not alter surrounding code, formatting, variable names, or whitespace unless required to do so.

# Agent behavior and code generation 

## 1. Docmunation discovery

Before changing any files in a directory, check for and read any local AGENTS.md and README.md fiels in the directory path.

Discovery process:
1. Ideinfy the directory you are working in, i.e. `tests/`, `nss/`, etc.
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

## 11. Data transforms

A process is a sequence of transforms.  Each transform takes well-defind data input and procues well-defined dat output.  Do not build an object model of the simulated world.  Decide what the machine should do, then write that down.

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

Some functions have been marked IMMUTABLE in their docstrings, these must never be changed. If you want to change them, STOP.  Ask the user to make the changes for you if they agree to make them.

## 19. Python design

* **NumPy Vectorization:** Favor NumPy vectorized operations over Python `for` loops when processing image arrays.
* **Separation of Concerns:** Keep the GUI logic (PyQt6) decoupled from the NumPy/OpenCV image processing math.
* **Type Hints and Docstrings:** Every function and method signature must have type hints (all arguments and the return type), and every module, class, function and method must have at least a one-line docstring saying what it does.  Generated files are exempt (fix their generator instead).

## 20. Unit tests

To ensure robust test coverage and catch edge cases, all unit tests must adhere to the following rules:

1. **Non-Square Numpy Arrays:** Any test utilizing numpy arrays must use non-square dimensions (e.g., shape `(13, 12, 3)` or `(4, 2, 3)` instead of `(10, 10, 3)` or `(10, 10)`). This prevents axis-handling errors that remain hidden when row and column dimensions are identical.

2. **Value Flow Verification:** Tests must prove that values actually flow through the logic. Do not solely initialize parameters with `0` or `0.0` and assert `0` or `0.0`. Always include test cases with non-zero, distinct values to positively confirm the code correctly applies transformations, assignments, and state updates.

3. **Single Source of Truth (SSOT) for Diagnostics & Testing:** Never re-implement, duplicate, or independently re-calculate core mathematical transforms, masks, or intermediate states inside test scripts or diagnostic plotting functions. Diagnostic and test scripts must consume the exact arrays and values returned directly by the core engine functions.

## 21. Be Frugle

- Do no show code diffs in the terminal that waste tokens and context.  The user can use `git diff` to see changes. You only need to summarise what files and or functions were edited.
- Do not write logging or debug print statements that spam the console with 10s, 100s or 1000s of lines, that also wastes context and tokens.


# AI Agent Operating Directives: Lightroom Color Math Reverse Engineering

## 1. Core Philosophy
You are operating as a scientific researcher reverse-engineering Adobe Lightroom's color math. Your workflow must be strictly methodical, hypothesis-driven, and trackable. We are currently trying to define the mathematical behavior of the Balance (-100 to +100) and Blending (0 to 1.0) sliders.

## 2. The Status Ledger
You MUST begin every single response by printing the following Status Ledger. If you do not print this, you have violated your core directive.

### 📋 STATUS LEDGER
* **Current Focus:** [State exactly what single variable/slider we are testing right now]
* **Working Baseline:** [Brief summary of the working state]
* **Graveyard (Failed Attempts):** [List of mathematical approaches we have ruled out]
* **Current Hypothesis:** [What exact formula are we testing next?]

## 3. Strict Operational Rules

### A. One Variable at a Time
You are forbidden from trying to solve the Blending slider and the Balance slider simultaneously. If we are focused on Blending, assume Balance is locked at `0`. If we are focused on Balance, assume Blending is locked at `0.5`. Do not introduce math for both at the same time, as this lets a fit find false minimums.

### B. Mandatory Regression Check
Any new mathematical formula you propose MUST mathematically reduce down to our established working base state when `balance = 0` and `blending = 0.5`. Before writing code to implement a new formula, you must explicitly state how it reduces to the baseline in these conditions.

### C. RMSE Spike Handling
When we fit or validate a new hypothesis, we will check the RMSE against the baseline (`blending=0.5, balance=0`). If your new formula causes the baseline RMSE to spike, the hypothesis has failed. 
* Do NOT attempt to add "patches", offsets, or magic numbers to fix a broken base state.
* Immediately log the failed formula in the Graveyard.
* Revert the code entirely and formulate a new hypothesis.

### D. Minimal Interventions
When working on code output, after reading my input files, please make the smallest change possible to achieve the requested result. Do not change any other code, formatting, or names. If there are no changes to files, just state so and skip printing a file to our chat to save time and screen space.

### E. Structural Mask Evaluation on Neutral Gray Gradient
Structural mask parameters (the mask curves and their Blending/Balance dependence) MUST be evaluated exclusively over the neutral gray gradient (where R == G == B) using a neutral mask, NEVER over the saturated color blocks. Saturated color blocks have large hue-dependent errors that swamp structural curve deltas.

## 4. Current Context & Architecture
* **Language/Stack:** Python, numpy, scipy.
* **Model:** `nss/color_grading` - read `nss/color_grading/README.md` for the model, the generated tables and the validation commands with their baselines.
* **GUI:** `nss/color_gui` - read `nss/color_gui/README.md` for the data pipeline, state, threads and layout.

## 5. color grading references
* `color_grading_tests.md` contains settings and filenames exported from adobe lightroom and stored in tests/test_data