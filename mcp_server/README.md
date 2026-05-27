# EZCAP MCP Server

This MCP server forwards tool calls to the EZCAP local IPC channel.

## Prerequisites

- Start EZCAP with IPC enabled:
  - Environment variable: `EZCAP_MCP_IPC=1`
  - Or command line: `--mcp-ipc`
- Ensure Node.js 18+ is installed.

## Install & Run

```
cd mcp_server
npm install
npm start
```

## HTTP Mode

```
cd mcp_server
npm install
npm run start:http
```

Defaults:

- URL: `http://127.0.0.1:3333/mcp`
- Health: `http://127.0.0.1:3333/healthz`

Environment overrides:

- `EZCAP_MCP_HTTP_PORT` (default `3333`)
- `EZCAP_MCP_HTTP_PATH` (default `/mcp`)
- `EZCAP_MCP_TOKEN` (optional `Authorization: Bearer <token>`)

## Pipe Configuration

Default pipe name:

- Windows: `\\.\pipe\ezcap_mcp`
- macOS/Linux: `/tmp/ezcap_mcp`

Override with `EZCAP_MCP_PIPE`.

## Supported Tools

- `app.ping`
- `app.info`
- `camera.status`
- `help.methods`

## Notes

- This server uses a simple JSON-RPC bridge to EZCAP IPC.
- Extend tool list by adding methods to EZCAP IPC and mapping them here.
