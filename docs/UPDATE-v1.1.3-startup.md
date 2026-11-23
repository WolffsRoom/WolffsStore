# Wolff's Store v1.1.3 — startup animation (test build)

## New

- Original **Wolff's Room** logo falls in three exact pixel layers: **3 → 2 → 1**.
- Native 8-bit falling and impact sound effects, following the Store's sound settings.
- All logo layers are resized to **50% (393×54)** for the **960×544** PlayStation Vita screen.
- Black opening screen with a separate pixel-framed catalog update status and real progress.
- Existing localized rotating wait messages appear after **3.4 seconds** if the network is still busy.
- Additional longer-than-usual notice appears after **8 seconds**.
- Original typewriter/home intro begins after the animation **and** after catalog sync has finished (including offline fallback).
- Updated LiveArea startup.png with the same original layered logo, adapted to **280×158 indexed PNG** with transparency.
- Catalog generator now rebuilds the three startup WRT sprites when regenerating catalog.

## Validation

- 97 existing/added Python tests passed (one skipped).
- Release toolchain: VitaSDK Docker atamanenko/vitasdk-softfp:latest.
- Pending: smoke test on physical PS Vita (sync success, slow network, offline startup, volume off/on, LiveArea artwork and performance).

This is a **test build**, not a GitHub release. Publish only after hardware verification.
