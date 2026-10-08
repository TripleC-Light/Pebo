"""Pebo read-only MCP server.

Run over stdio. Requires the official Python MCP SDK:
    python -m pip install mcp
"""

from __future__ import annotations

from typing import Any, Dict, List

from mcp.server.fastmcp import FastMCP

from pebo_bridge_core import (
    BridgeError,
    list_project_files,
    project_status,
    read_project_file,
)


mcp = FastMCP("pebo-readonly-bridge")


def _error(message: str) -> Dict[str, Any]:
    return {"ok": False, "error": message}


@mcp.tool()
def get_project_status() -> Dict[str, Any]:
    """Return Pebo project structure and development status."""
    return {"ok": True, "status": project_status()}


@mcp.tool()
def list_files(relative_path: str = ".") -> Dict[str, Any]:
    """List allowed Pebo project files under AGENTS, README, docs, or Firmware."""
    try:
        return {"ok": True, "files": list_project_files(relative_path)}
    except BridgeError as exc:
        return _error(str(exc))


@mcp.tool()
def read_file(relative_path: str, max_bytes: int = 200_000) -> Dict[str, Any]:
    """Read an allowed Pebo project text/source file."""
    try:
        return {"ok": True, "file": read_project_file(relative_path, max_bytes=max_bytes)}
    except BridgeError as exc:
        return _error(str(exc))


@mcp.resource("pebo://status")
def status_resource() -> Dict[str, Any]:
    """Pebo read-only project status resource."""
    return project_status()


@mcp.resource("pebo://file/{relative_path}")
def file_resource(relative_path: str) -> str:
    """Read an allowed Pebo project file as a resource."""
    return read_project_file(relative_path)["content"]


def main() -> None:
    mcp.run()


if __name__ == "__main__":
    main()
