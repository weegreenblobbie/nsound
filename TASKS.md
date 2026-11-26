Claude / Gemini and I will work on the following.

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

[x] How can we test audio playback in linux bazel test builds?
[x] Update the windows bazel build so AudioPlayback works
[x] Can audio playback be tested with bazel test on Windows too?

Remove all C++ build warnings
-----------------------------

[x] Treat warnings as errors

Dockerfile and start-agent
--------------------------

[x] Update Dockerfile to incoporate Dockerfile.gemini to build a claude and gemini docker container
[x] Update start-claude.ps1 and start-gemini.ps1 to a single start-agent.psy that takes [calude|gemini] [resume-hash]

Chords in bebot GUI
-------------------

[x] Update the bebot.py GUI demo with the following:

Inspect the existing Pygame UI demo in this workspace first and summarize what files, controls, and audio/synthesis setup are already there before making edits.

Build an interactive X-Y chord/arpeggio synthesizer GUI on top of the Pygame UI:

1. X-Y Pad Interaction (Mouse Press, Drag, Release):
   - Keep the square X-Y pad where pressing and holding the mouse button starts audio generation, moving the mouse updates the sound in real time, and releasing stops/fades it cleanly.
   - Vertical axis (Y): maps to root pitch (continuous glide by default, or snapped to semitones when Snap to Semitones is enabled), with all chord voices moving together in lockstep off the shifting root frequency f_root = 440 * 2^((m_root - 69) / 12) and f_n = f_root * 2^(n / 12).
   - Horizontal axis (X): maps smoothly to a real-time synthesizer parameter (such as filter cutoff or harmonic brightness/timbre) that complements chord playback.
   - Draw visual feedback on the X-Y pad, including live cursor/crosshair position and faint horizontal semitone guide lines when semitone snapping is active.

2. Compact Chord Selector:
   - Add a small chord-type dropdown/selector (kept compact around ~6–8 qualities such as Major, Minor, Sus2, Sus4, Dim, Aug, Maj7, Min7, using semitone offsets from the root) rather than listing every root+chord combination.

3. Semitone Snap Toggle & Arpeggio Mode:
   - Add a "Snap to Semitones" toggle to switch between continuous frequency gliding and quantized semitone steps.
   - Add an "Arpeggio" toggle with a "Speed" parameter (e.g., Hz or BPM) so when enabled, it steps through the active chord's notes sequentially on repeat while the mouse is held (resetting cleanly to the root on mouse-down), and when disabled, it plays the chord voices together.

4. Clean Real-Time Audio & Thread Safety:
   - Ensure parameter updates from the Pygame event loop to the audio engine are thread-safe, scaled cleanly (e.g., dividing per-voice gain by voice count to prevent clipping), and smoothed with short ramps on note/step/parameter changes to avoid clicks, pops, or zipper noise.
   - Keep changes focused, modular, and easy to run/test from the command line, and report any new dependencies or run commands needed.

Publish to pypi
---------------
[ ] Publish a new nsound pyton package on pypi
