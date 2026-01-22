#!/usr/bin/env python3
"""
Ensure the documented helper ABI mirrors the headers.

This script parses `include/ternary_runtime.h` (and any extra helper headers)
for `__ternary_*` symbols and confirms `SPECIFICATION.md` or `README.md`
already list each helper name. The checklist prevents the documentation
from drifting whenever new helpers are added.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path


BASE_DIR = Path(__file__).resolve().parent.parent
HEADER_FILES = [
    BASE_DIR / "include" / "ternary_runtime.h",
    BASE_DIR / "include" / "ternary_helpers.h",
]
DOC_FILES = [
    BASE_DIR / "SPECIFICATION.md",
    BASE_DIR / "README.md",
]

PATTERN = re.compile(r"(__ternary_[A-Za-z0-9_]+)\s*\(")
DOC_PATTERN = re.compile(r"__ternary_[A-Za-z0-9_]+")


def extract_symbols(path: Path) -> set[str]:
    text = path.read_text(encoding="utf-8")
    return set(PATTERN.findall(text))


def main() -> int:
    header_symbols: set[str] = set()
    for header in HEADER_FILES:
        if not header.exists():
            print(f"warning: skipping missing header {header}", file=sys.stderr)
            continue
        header_symbols.update(extract_symbols(header))

    if not header_symbols:
        print("error: no helper symbols found in headers", file=sys.stderr)
        return 1

    doc_text = []
    for doc in DOC_FILES:
        if not doc.exists():
            continue
        doc_text.append(doc.read_text(encoding="utf-8"))
    if not doc_text:
        print("error: documentation files missing", file=sys.stderr)
        return 1

    doc_symbols = set()
    for text in doc_text:
        doc_symbols.update(DOC_PATTERN.findall(text))

    missing = sorted(header_symbols - doc_symbols)
    if missing:
        print("documentation is missing the following helper symbols:")
        for name in missing:
            print(f"  - {name}")
        print(
            "\nUpdate SPECIFICATION.md/README.md (and any linked documentation) "
            "so the helper list references these symbols."
        )
        return 1

    print("helper documentation matches headers")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
