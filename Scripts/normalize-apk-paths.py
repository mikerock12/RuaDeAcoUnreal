#!/usr/bin/env python3
"""Write portable APK entry names and verify every packaged asset against source.

aapt2 on Windows can emit backslashes in ZIP names. Windows zipfile normalizes
ZipInfo.filename while reading, concealing names Android cannot resolve. Always
inspect orig_filename, and rewrite both local and central directory headers.
Run before zipalign and signing; signed APKs must never be rewritten.
"""
from __future__ import annotations

import argparse
import hashlib
import shutil
import zipfile
from pathlib import Path


def normalize(source: Path, target: Path, assets: Path) -> None:
    if source.resolve() == target.resolve():
        raise ValueError("Input and output APK must differ")
    names: set[str] = set()
    changed = 0
    with zipfile.ZipFile(source) as src, zipfile.ZipFile(target, "w") as dst:
        for entry in src.infolist():
            raw = entry.orig_filename
            name = raw.replace("\\", "/")
            if name in names or name.startswith("/") or ".." in name.split("/"):
                raise ValueError(f"Duplicate or unsafe APK path: {name}")
            names.add(name)
            changed += name != raw
            portable = zipfile.ZipInfo(name, entry.date_time)
            portable.compress_type = entry.compress_type
            portable.external_attr = entry.external_attr
            portable.internal_attr = entry.internal_attr
            portable.create_system = entry.create_system
            # Do not copy extra fields containing stale offsets or size records.
            with src.open(entry) as reader, dst.open(portable, "w") as writer:
                shutil.copyfileobj(reader, writer, 1024 * 1024)
    expected = {"assets/" + p.relative_to(assets).as_posix(): p
                for p in assets.rglob("*") if p.is_file() and p.name != ".gitkeep"}
    if not expected:
        raise ValueError("No source assets to verify")
    with zipfile.ZipFile(target) as apk:
        if any("\\" in entry.orig_filename for entry in apk.infolist()):
            raise ValueError("Nonportable raw APK path remains")
        actual = {entry.orig_filename for entry in apk.infolist()
                  if not entry.is_dir() and entry.orig_filename.startswith("assets/")}
        if actual != set(expected):
            raise ValueError(f"Asset path mismatch: missing={set(expected)-actual}, extra={actual-set(expected)}")
        for name, path in expected.items():
            if hashlib.sha256(apk.read(name)).digest() != hashlib.sha256(path.read_bytes()).digest():
                raise ValueError(f"Asset bytes differ: {name}")
        bad = apk.testzip()
        if bad:
            raise ValueError(f"APK CRC failed: {bad}")
    print(f"APK portable paths: normalized={changed}, verified assets={len(expected)} (SHA-256 and CRC)")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("target", type=Path)
    parser.add_argument("--assets", required=True, type=Path)
    args = parser.parse_args()
    normalize(args.source, args.target, args.assets)
