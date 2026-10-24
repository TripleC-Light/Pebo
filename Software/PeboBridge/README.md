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

Python 3.10+ and the official Python MCP SDK 1.x. The bridge currently uses
`mcp.server.fastmcp.FastMCP`; SDK 2.x changes that API. Install the bounded
requirements using the same interpreter that will launch the server:

```powershell
python -m pip install -r Software/PeboBridge/requirements.txt
```

Use a machine-local virtual environment. Interpreter paths and installation
status are recorded per computer in `docs/development-machines.md`.

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
