#!/usr/bin/env python3
"""Real APK UI smoke: rendered menus/portraits/HUD, pause and background/resume.

Requires adb and Pillow. Input injection and frame changes are recorded; these
pixel checks do not establish gameplay parity, physical ARM64 QA, multitouch,
audio-output quality or performance/latency. APK signing uses a separate QA key.
"""
from __future__ import annotations

import argparse
import hashlib
import io
import json
import re
import subprocess
import time
import traceback
import zipfile
import xml.etree.ElementTree as ET
from pathlib import Path
from PIL import Image, ImageChops, ImageStat

PACKAGE = "com.smellsliketech.ruadeaco"
COMPONENT = PACKAGE + "/.MainActivity"
FIGHTERS = ["guto-barba", "rafa-mare", "noir-reflexo", "astro-riso", "dante-sinal", "leo-violeta"]


class AndroidQA:
    def __init__(self, output: Path, serial: str | None):
        self.output = output
        output.mkdir(parents=True, exist_ok=True)
        self.prefix = ["adb"] + (["-s", serial] if serial else [])
        self.trace: list[dict] = []
        self.captures: list[dict] = []
        self.width, self.height = 1280, 720

    def adb(self, *args: str, check: bool = True, timeout: int = 45) -> bytes:
        result = subprocess.run(self.prefix + list(args), capture_output=True, timeout=timeout)
        self.trace.append({"command": list(args), "returncode": result.returncode})
        if check and result.returncode:
            raise AssertionError(f"adb {args[0]} failed: {result.stderr.decode(errors='replace')}")
        return result.stdout

    def shell(self, *args: str, **kwargs) -> str:
        return self.adb("shell", *args, **kwargs).decode("utf-8", errors="replace")

    def point(self, x: float, y: float) -> tuple[str, str]:
        scale = min(self.width / 640, self.height / 360)
        return (str(round((self.width - 640 * scale) / 2 + x * scale)),
                str(round((self.height - 360 * scale) / 2 + y * scale)))

    def tap(self, x: float, y: float, wait: float = 0.65):
        self.shell("input", "tap", *self.point(x, y))
        time.sleep(wait)

    def hold(self, x: float, y: float, milliseconds: int):
        point = self.point(x, y)
        self.shell("input", "swipe", *point, *point, str(milliseconds))
        time.sleep(0.3)

    def image(self) -> Image.Image:
        image = Image.open(io.BytesIO(self.adb("exec-out", "screencap", "-p"))).convert("RGB")
        self.width, self.height = image.size
        assert self.width > self.height, f"Expected landscape, got {image.size}"
        return image

    @staticmethod
    def logical(image: Image.Image) -> Image.Image:
        scale = min(image.width / 640, image.height / 360)
        left, top = (image.width - 640 * scale) / 2, (image.height - 360 * scale) / 2
        return image.crop((round(left), round(top), round(left + 640 * scale), round(top + 360 * scale))).resize((640, 360), Image.Resampling.NEAREST)

    def capture(self, name: str) -> Image.Image:
        image = self.image()
        logical = self.logical(image)
        target = self.output / (name + ".png")
        image.save(target)
        self.captures.append({"file": target.name, "bytes": target.stat().st_size,
                              "sha256": hashlib.sha256(target.read_bytes()).hexdigest(),
                              "width": image.width, "height": image.height})
        self.assert_rendered(logical)
        self.assert_alive()
        return logical

    @staticmethod
    def assert_rendered(image: Image.Image):
        sample = image.resize((160, 90))
        nonblack = sum(max(pixel) > 25 for pixel in sample.getdata()) / (160 * 90)
        assert nonblack > 0.08, f"Black/empty frame: {nonblack:.3f} visible pixels"
        assert max(ImageStat.Stat(sample).stddev) > 10, "Uniform frame instead of rendered frontend"

    def assert_alive(self) -> str:
        pid = self.shell("pidof", PACKAGE, check=False).strip()
        assert pid, "Native game process exited"
        return pid

    @staticmethod
    def cyan_fraction(image: Image.Image, box: tuple[int, int, int, int]) -> float:
        pixels = list(image.crop(box).getdata())
        return sum(r < 120 and g > 140 and b > 150 for r, g, b in pixels) / len(pixels)

    def assert_match(self, image: Image.Image):
        assert self.cyan_fraction(image, (24, 22, 220, 29)) > 0.35, "P1 life HUD absent; match did not open"
        pixels = list(image.crop((420, 22, 610, 29)).getdata())
        assert sum(r > 150 and g < 140 and b > 60 for r, g, b in pixels) / len(pixels) > 0.3, "P2 life HUD absent"

    def assert_pause(self, image: Image.Image, expected: bool):
        fraction = self.cyan_fraction(image, (140, 92, 142, 272))
        assert (fraction > 0.55) == expected, f"Pause overlay expected={expected}, cyan border fraction={fraction:.3f}"

    def wait_menu(self):
        deadline = time.monotonic() + 50
        onboarding_checks = 0
        while time.monotonic() < deadline:
            self.assert_alive()
            try:
                image = self.logical(self.image())
                self.assert_rendered(image)
                if self.cyan_fraction(image, (160, 120, 162, 298)) > 0.3:
                    return
            except (AssertionError, OSError):
                pass
            if onboarding_checks < 3:
                self.dismiss_fullscreen_hint()
                onboarding_checks += 1
            time.sleep(1)
        raise AssertionError("Rendered main menu did not appear within 50 seconds")

    def dismiss_fullscreen_hint(self):
        """Only dismiss an identified Android SystemUI onboarding button by its bounds."""
        try:
            self.shell("uiautomator", "dump", "/sdcard/rda-qa-window.xml", timeout=15)
            xml = self.adb("exec-out", "cat", "/sdcard/rda-qa-window.xml", timeout=15)
            tree = ET.fromstring(xml)
        except (ET.ParseError, subprocess.TimeoutExpired, AssertionError):
            return
        for node in tree.iter("node"):
            if node.get("package") != "com.android.systemui":
                continue
            text = (node.get("text", "") or node.get("content-desc", "")).strip().lower()
            if text not in {"got it", "ok", "entendi"}:
                continue
            match = re.fullmatch(r"\[(\d+),(\d+)\]\[(\d+),(\d+)\]", node.get("bounds", ""))
            if not match:
                continue
            x1, y1, x2, y2 = map(int, match.groups())
            if x2 <= x1 or y2 <= y1:
                continue
            self.shell("input", "tap", str((x1 + x2) // 2), str((y1 + y2) // 2))
            self.trace.append({"uiAction": "dismiss-known-fullscreen-hint", "package": node.get("package"), "text": text, "bounds": node.get("bounds")})
            time.sleep(0.4)
            return

    def run(self, apk: Path):
        with zipfile.ZipFile(apk) as package:
            raw_names = [entry.orig_filename for entry in package.infolist()]
            assert not any("\\" in name for name in raw_names), "APK contains paths Android cannot resolve"
            assert "assets/Content/RuaDeAco/Art/UI/rua-de-aco-logo.png" in raw_names, "Canonical APK logo missing"
            assert sum(name.startswith("assets/") and not name.endswith("/") for name in raw_names) == 338, "Canonical APK must contain all 338 original assets"
            assert package.testzip() is None, "APK CRC check failed"
        self.adb("wait-for-device", timeout=90)
        self.shell("wm", "size", "1280x720")
        self.shell("wm", "density", "160")
        self.shell("settings", "put", "system", "accelerometer_rotation", "0")
        self.shell("settings", "put", "system", "user_rotation", "0")
        self.shell("settings", "put", "global", "policy_control", "immersive.full=" + PACKAGE)
        self.shell("settings", "put", "secure", "immersive_mode_confirmations", "confirmed")
        self.adb("install", "-r", str(apk.resolve()), timeout=120)
        self.adb("logcat", "-c")
        device = {"abi": self.shell("getprop", "ro.product.cpu.abi").strip(),
                  "api": self.shell("getprop", "ro.build.version.sdk").strip(),
                  "model": self.shell("getprop", "ro.product.model").strip(),
                  "physicalDevice": False, "gpu": "CI emulator SwiftShader"}
        assert device["abi"] == "x86_64", "QA artifact must run on the x86_64 emulator"
        portrait_checks = []
        for fighter, name in enumerate(FIGHTERS):
            self.shell("am", "force-stop", PACKAGE)
            self.shell("am", "start", "-W", "-n", COMPONENT)
            self.wait_menu()
            self.capture(f"{fighter:02}-{name}-00-title")
            self.tap(320, 218)  # Training; stationary opponent for this UI smoke.
            selection = self.capture(f"{fighter:02}-{name}-01-selection")
            for index in range(6):
                crop = selection.crop((33 + 98 * index, 54, 108 + 98 * index, 125))
                assert max(ImageStat.Stat(crop).stddev) > 12, f"Missing/blank portrait {index}"
            portrait_checks.append({"p1": name, "sixPortraitsRendered": True})
            self.tap(71 + 98 * fighter, 105)
            self.capture(f"{fighter:02}-{name}-02-p1-chosen")
            self.tap(71 + 98 * ((fighter + 1) % 6), 105)
            self.capture(f"{fighter:02}-{name}-03-opponent-chosen")
            self.tap(121 + 198 * (fighter % 3), 300, wait=4)
            before = self.capture(f"{fighter:02}-{name}-04-match")
            self.assert_match(before)
            self.hold(106, 294, 650)
            self.hold(72, 260, 160)
            self.tap(536, 294)
            self.tap(572, 268)
            self.tap(604, 294)
            after = self.capture(f"{fighter:02}-{name}-05-after-input")
            self.assert_match(after)
            changed = sum(max(pixel) > 12 for pixel in ImageChops.difference(before, after).crop((0, 60, 640, 280)).getdata())
            assert changed > 100, "Gameplay frame did not update after injected input"
            self.tap(614, 20)
            paused = self.capture(f"{fighter:02}-{name}-06-paused")
            self.assert_pause(paused, True)
            self.tap(320, 130)
            resumed = self.capture(f"{fighter:02}-{name}-07-resumed")
            self.assert_match(resumed)
            self.assert_pause(resumed, False)
            pid = self.assert_alive()
            self.shell("input", "keyevent", "KEYCODE_HOME")
            time.sleep(1)
            self.shell("am", "start", "-W", "-n", COMPONENT)
            time.sleep(3)
            assert self.assert_alive() == pid, "Process restarted during short background/resume"
            lifecycle = self.capture(f"{fighter:02}-{name}-08-background-resume")
            self.assert_match(lifecycle)
            print(f"Android emulator UI smoke: {name}, arena {fighter % 3}, input/pause/resume captured", flush=True)
        return {"device": device, "fighters": portrait_checks, "arenasVisited": 3,
                "inputsInjected": ["right", "jump", "light", "heavy", "special"],
                "pauseAndResumeObserved": True, "backgroundRetainedMatch": True,
                "limits": ["No physical ARM64 devices", "No audio-output measurement", "No multitouch assertion",
                           "No gameplay parity or performance/latency conclusion from pixel smoke"]}

    def diagnostics(self):
        for filename, args in [("logcat.txt", ("logcat", "-d", "-v", "threadtime")),
                               ("crash-buffer.txt", ("logcat", "-b", "crash", "-d")),
                               ("activity.txt", ("shell", "dumpsys", "activity", "activities")),
                               ("memory.txt", ("shell", "dumpsys", "meminfo", PACKAGE))]:
            try:
                (self.output / filename).write_bytes(self.adb(*args, check=False, timeout=20))
            except Exception as error:
                (self.output / filename).write_text(str(error), encoding="utf-8")
        (self.output / "input-trace.json").write_text(json.dumps(self.trace, indent=2), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--apk", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--serial")
    args = parser.parse_args()
    qa = AndroidQA(args.output, args.serial)
    result = {"passed": False}
    try:
        result.update(qa.run(args.apk))
        result["passed"] = True
    except Exception as error:
        result["error"] = str(error)
        (args.output / "failure.txt").write_text(traceback.format_exc(), encoding="utf-8")
        try:
            qa.capture("failure-screen")
        except Exception:
            pass
    finally:
        qa.diagnostics()
        crash = (args.output / "crash-buffer.txt").read_text(encoding="utf-8", errors="replace")
        if PACKAGE in crash and re.search(r"Fatal signal|FATAL EXCEPTION|Abort message:", crash):
            result.update(passed=False, crashBufferFailure=True)
        result["captures"] = qa.captures
        (args.output / "result.json").write_text(json.dumps(result, indent=2), encoding="utf-8")
    print(json.dumps({key: value for key, value in result.items() if key != "captures"}, indent=2), flush=True)
    return 0 if result["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
