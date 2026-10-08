"""Read-only Pebo project bridge core.

This module intentionally has no MCP dependency so path-safety behavior can be
tested even when the MCP SDK is not installed yet.
"""

from __future__ import annotations

import os
from pathlib import Path
from typing import Dict, Iterable, List


PROJECT_ROOT = Path(__file__).resolve().parents[2]

ALLOWED_ROOTS = {
    "AGENTS.md",
    "README.md",
    "docs",
    "Firmware",
}

DENIED_NAMES = {
    ".env",
    ".git",
    ".hg",
    ".svn",
    ".codex",
    "auth.json",
    "credentials.json",
    "token.json",
    "secrets.json",
}

DENIED_SUFFIXES = {
    ".key",
    ".pem",
    ".p12",
    ".pfx",
}

ALLOWED_SUFFIXES = {
    "",
    ".c",
    ".cpp",
    ".h",
    ".hpp",
    ".ino",
    ".md",
    ".txt",
    ".json",
    ".toml",
    ".yaml",
    ".yml",
}


class BridgeError(ValueError):
    """Raised when a requested path or operation is outside the read-only scope."""


def _relative_parts(path: Path) -> Iterable[str]:
    try:
        relative = path.relative_to(PROJECT_ROOT)
    except ValueError as exc:
        raise BridgeError("Path is outside the Pebo project") from exc
    return relative.parts


def _is_denied_name(path: Path) -> bool:
    return any(part.lower() in DENIED_NAMES for part in path.parts)


def _is_allowed_top_level(path: Path) -> bool:
    parts = list(_relative_parts(path))
    if not parts:
        return True
    return parts[0] in ALLOWED_ROOTS


def resolve_project_path(relative_path: str) -> Path:
    if not relative_path or relative_path.strip() in {".", "./"}:
        return PROJECT_ROOT

    requested = Path(relative_path)
    if requested.is_absolute():
        raise BridgeError("Absolute paths are not allowed")

    candidate = (PROJECT_ROOT / requested).resolve(strict=False)
    _relative_parts(candidate)

    if not _is_allowed_top_level(candidate):
        raise BridgeError("Path is not in the allowed read-only project areas")

    if _is_denied_name(candidate) or candidate.suffix.lower() in DENIED_SUFFIXES:
        raise BridgeError("Refusing to read credential or hidden control files")

    return candidate


def assert_existing_safe_path(path: Path) -> Path:
    if not path.exists():
        raise BridgeError("Path does not exist")

    resolved = path.resolve(strict=True)
    _relative_parts(resolved)

    if not _is_allowed_top_level(resolved):
        raise BridgeError("Resolved path is not in the allowed read-only project areas")

    if _is_denied_name(resolved) or resolved.suffix.lower() in DENIED_SUFFIXES:
        raise BridgeError("Refusing to read credential or hidden control files")

    if resolved.is_symlink():
        raise BridgeError("Refusing to read symbolic links")

    return resolved


def project_status() -> Dict[str, object]:
    docs_dir = PROJECT_ROOT / "docs"
    firmware_dir = PROJECT_ROOT / "Firmware"
    software_dir = PROJECT_ROOT / "Software"
    git_dir = PROJECT_ROOT / ".git"

    docs = sorted(p.name for p in docs_dir.glob("*.md")) if docs_dir.exists() else []
    firmware = sorted(p.name for p in firmware_dir.iterdir()) if firmware_dir.exists() else []
    software = sorted(p.name for p in software_dir.iterdir()) if software_dir.exists() else []

    return {
        "project": "Pebo",
        "root": str(PROJECT_ROOT),
        "git_repository": git_dir.exists(),
        "allowed_roots": sorted(ALLOWED_ROOTS),
        "docs": docs,
        "firmware_entries": firmware,
        "software_entries": software,
        "notes": [
            "read-only MCP bridge",
            "no credential or environment-variable access",
            "no firmware upload, flashing, or hardware control",
        ],
    }


def list_project_files(relative_path: str = ".") -> List[Dict[str, object]]:
    root = assert_existing_safe_path(resolve_project_path(relative_path))
    if root.is_file():
        root = root.parent

    entries: List[Dict[str, object]] = []
    for item in sorted(root.iterdir(), key=lambda p: p.name.lower()):
        if _is_denied_name(item) or item.suffix.lower() in DENIED_SUFFIXES:
            continue
        try:
            resolved = item.resolve(strict=True)
            resolved.relative_to(PROJECT_ROOT)
        except (OSError, ValueError):
            continue
        if not _is_allowed_top_level(resolved):
            continue
        entries.append(
            {
                "name": item.name,
                "relative_path": str(resolved.relative_to(PROJECT_ROOT)),
                "type": "directory" if item.is_dir() else "file",
                "size": item.stat().st_size if item.is_file() else None,
            }
        )
    return entries


def read_project_file(relative_path: str, max_bytes: int = 200_000) -> Dict[str, object]:
    path = assert_existing_safe_path(resolve_project_path(relative_path))
    if path.is_dir():
        raise BridgeError("Path is a directory")
    if path.suffix.lower() not in ALLOWED_SUFFIXES:
        raise BridgeError("File type is not allowed")
    if path.stat().st_size > max_bytes:
        raise BridgeError("File is larger than max_bytes")

    data = path.read_text(encoding="utf-8", errors="replace")
    return {
        "relative_path": str(path.relative_to(PROJECT_ROOT)),
        "size": path.stat().st_size,
        "content": data,
    }


def main() -> None:
    import json

    print(json.dumps(project_status(), ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
