# Auto-update UI and Video Fix Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Ship v0.7 with a branded update flow, evenly spaced top actions, and Vita-compatible MP4 playback.

**Architecture:** Keep the existing screens and AvPlayer subsystem, replacing only layout constants, updater drawing, and the player resource/file boundary. Add source-level regression checks for the Vita-specific invariants and verify with the real Vita build.

**Tech Stack:** C, VitaSDK, vita2d, SceAvPlayer, CMake, PowerShell/Python source checks

**Spec:** `docs/superpowers/specs/2026-11-14-update-ui-video-fix-design.md`

## Global Constraints

- Installation progress must remain indeterminate.
- Download progress must remain real.
- Direct GPU playback remains hardware-decoded H.264/AAC up to 960x544.

## Review Focus

- GPU allocation is 1 MiB aligned and is always unmapped/freed.
- Custom file callback failure retries with native access.
- Frame texture is not repointed while rendering is in flight.
- Equal tray spacing fits inside the 960-pixel screen.
- Update UI remains legible in Portuguese without external font assets.

---

### Task 1: Regression checks

**Files:**
- Create: `tests/test_release_invariants.py`

- [ ] Write checks for v0.7, equal tray columns, branded updater, PHYCONT video buffers, file callbacks/fallback, and render synchronization.
- [ ] Run the test and verify it fails on the current v0.6 sources.

### Task 2: Update interfaces and tray

**Files:**
- Modify: `src/main.c`
- Modify: `updater/main.c`
- Modify: `CMakeLists.txt`

- [ ] Implement equal-width tray columns.
- [ ] Add the paw, bouncing Wolff's Room mark and improved real progress to the download overlay.
- [ ] Add the same identity and an honest indeterminate segmented bar to the installer.
- [ ] Bump the version to v0.7.

### Task 3: AvPlayer compatibility

**Files:**
- Modify: `src/video.c`

- [ ] Replace CDRAM allocation with the proven PHYCONT allocation and five buffers.
- [ ] Add VitaShell-style file callbacks and native fallback.
- [ ] Synchronize rendering before repointing the decoded-frame texture.
- [ ] Run the regression checks and verify they pass.

### Task 4: Verification and publication

- [ ] Build the Vita VPK from a clean configure.
- [ ] Inspect the VPK version and updater payload.
- [ ] Review the final diff and run `git diff --check`.
- [ ] Commit, push main, tag v0.7, watch the release workflow, and verify the published VPK.
