# Video player (experimental Vita build)

The store supports post videos using locally cached MP4 files. The site exposes the full
URL in store.json under the post's video_arquivo field. The bundled offline catalog
also points Pizza Tower and Deltarune at their Vita-targeted v2 files.

Flow: tap video -> download .part -> atomically rename -> native SceAvPlayer
-> GXM NV12 sampler -> Vita2D fullscreen rendering. Five physically contiguous
frame buffers are allocated by the decoder. If native file access fails, the
existing custom ux0 file callbacks are attempted. Invalid cached media is
downloaded again once; subsequent decode failure is reported, not hidden.

The native local-file mode follows the general mechanism in the GPLv3
VitaDB Downloader player (Rinnegatamante); this implementation is written
for the Store's Vita2D renderer and uses SceAvPlayer's native audio for local files.

The app's usual BGM is muted while a video is open and restored after closing.
X = pause/resume, left/right = seek 10 seconds, circle = close.

Logs include AvPlayerInit, AddSource, GPU allocation and first-frame events.
When testing on real hardware, inspect ux0:data/WolffsStore/logs/wolffsstore.log for "video:".

Known limitation: Vita3K does not fully emulate the native Vita multimedia
pipeline. Compilation and static tests are not proof of hardware playback.
Validate audio + video + seeking on a Vita before publishing a release.
