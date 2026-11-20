"""Run functional taiHEN installer tests against the real C source.

Uses host GCC on Linux/macOS; on Windows, uses the preinstalled VitaSDK
Docker image, where GCC is available. Does not access any PS Vita device.
"""
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ("src/touch_plugin.c", "tests/host_touch_plugin.c")
ARGS = ("-std=c11", "-O1", "-Wall", "-Wextra", "-Werror",
        "-Itests/host_stubs", "-Isrc", *SRC, "-o", "/tmp/ws_touch_host")

def run():
    gcc = shutil.which("gcc")
    if gcc and sys.platform != "win32":
        commands = [
            [gcc, *ARGS],
            ["/tmp/ws_touch_host"],
        ]
    elif shutil.which("docker"):
        mount = str(ROOT).replace("\\", "/")
        expression = "gcc " + " ".join(ARGS) + " && /tmp/ws_touch_host"
        commands = [[
            "docker", "run", "--rm", "-v", mount + ":/project",
            "-w", "/project", "atamanenko/vitasdk-softfp:latest",
            "sh", "-lc", expression,
        ]]
    else:
        print("No GCC or Docker available for functional taiHEN tests.", file=sys.stderr)
        return 2
    for cmd in commands:
        ret = subprocess.run(cmd, cwd=ROOT, check=False).returncode
        if ret:
            return ret
    return 0

if __name__ == "__main__":
    raise SystemExit(run())
