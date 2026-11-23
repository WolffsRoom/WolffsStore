# Wolff's Store v1.1.3 — startup animation (test build)

## New

- Original **Wolff's Room** logo falls in three exact pixel layers: **3 → 2 → 1**.
- Native 8-bit falling and impact sound effects, following the Store's sound settings.
- All logo layers are resized to **50% (393×54)** for the **960×544** PlayStation Vita screen.
- Black opening screen; after the original layers finish falling, the Store's **original unframed bouncing "Updating the Store..." notice** appears.
- Existing localized humorous rotating wait messages appear **3.4 seconds after the bouncing notice begins**, only while sync is running.
- The animation is uninterrupted and the bouncing notice remains on screen at least 0.85 seconds before the usual title sequence.
- Original typewriter/home intro begins after the animation **and** after catalog sync has finished (including offline fallback).
- LiveArea **startup.png copied exactly from the user-selected root assets/livearea/startup.png**, without redesign, at **280×158 indexed PNG** with transparency.
- Catalog generator now rebuilds the three startup WRT sprites when regenerating catalog.

## Validation

- 97 existing/added Python tests passed (one skipped).
- Release toolchain: VitaSDK Docker atamanenko/vitasdk-softfp:latest.
- Pending: smoke test on physical PS Vita (sync success, slow network, offline startup, volume off/on, LiveArea artwork and performance).

This is a **test build**, not a GitHub release. Publish only after hardware verification.
