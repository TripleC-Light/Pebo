# Pebo read-only MCP bridge

Minimal stdio MCP server for the local Pebo project.

## Scope

Allowed:

- Project status
- Directory listing under allowed project areas
- Reading `AGENTS.md`, `README.md`, `docs/`, and firmware source files

Denied:

- Paths outside `G:\Google Drive\Application\Project\Pebo`
- Path traversal
- Symlink targets that resolve outside the project
- Credential-like filenames such as `auth.json`, `.env`, `*.pem`, `*.key`
- Environment variable access
- Firmware upload, flashing, hardware control, or file modification

## Requirements

Python 3.9+ and the official Python MCP SDK:

```powershell
python -m pip install mcp
```

In this environment, pip install currently fails due local TLS/transfer issues,
so the MCP server source is present but the stdio server cannot be launched
until the SDK is installed.

## Stdio command

For tunnel-client or an MCP client configured for stdio:

```powershell
python "G:\Google Drive\Application\Project\Pebo\Software\PeboBridge\server.py"
```

## Tools

- `get_project_status`
- `list_files`
- `read_file`

## Resources

- `pebo://status`
- `pebo://file/{relative_path}`

## Local tests

Core path-safety tests:

```powershell
python -m unittest discover -s Software\PeboBridge\tests
```
