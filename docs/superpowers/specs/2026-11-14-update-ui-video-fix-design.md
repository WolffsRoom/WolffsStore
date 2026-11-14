# Auto-update UI and Video Fix Design

## Intent

Make the v0.7 update flow feel like Wolff's Store and make locally cached MP4 playback follow proven PS Vita AvPlayer patterns.

## Decisions

- The main app's update download overlay uses the pixel paw, a centered bouncing `WOLFF'S ROOM` wordmark, explicit version text, and a framed segmented progress bar with a real percentage.
- The standalone installer uses the same identity, but its progress remains indeterminate because PromoterUtil exposes no reliable percentage. A moving segmented bar communicates activity without inventing progress.
- Top-tray actions use equal-width columns and equal gaps. Labels may overflow their focused column visually; hidden labels no longer influence spacing.
- Video uses the VitaGL sample's physically contiguous, 1 MiB-aligned GPU frame allocation with five frame buffers. File callbacks follow the working VitaShell/FMVita pattern and retry once with native file handling if a firmware rejects callbacks.
- Rendering waits for the previous GPU use before repointing the texture to a new decoded frame.
- Version becomes v0.7.

## Constraints

- Keep MP4 support at H.264 + AAC, up to 960x544.
- Preserve current download/cache behavior and controller controls.
- Do not show a fabricated installation percentage.

## Approaches considered

- Codec-only re-encoding: rejected because the published files already validate against the requested codec limits.
- CPU YUV conversion: robust but unnecessarily expensive for 960x544 playback.
- Native AvPlayer/GXM path matching VitaGL and VitaShell: selected because it addresses allocation, file access and GPU synchronization while retaining hardware decoding.
