import express from "express";
import { callIpc } from "./ipcClient.js";

const app = express();
app.use(express.json({ limit: "1mb" }));

const PORT = Number.parseInt(process.env.EZCAP_MCP_HTTP_PORT || "3333", 10);
const PATH = process.env.EZCAP_MCP_HTTP_PATH || "/mcp";
const AUTH_TOKEN = process.env.EZCAP_MCP_TOKEN || "";

function isAuthorized(req) {
  if (!AUTH_TOKEN) {
    return true;
  }
  const header = req.headers.authorization || "";
  return header === `Bearer ${AUTH_TOKEN}`;
}

function jsonRpcError(id, code, message) {
  return {
    jsonrpc: "2.0",
    id: id ?? null,
    error: {
      code,
      message,
    },
  };
}

function toolsListResult() {
  return {
    tools: [
      {
        name: "app.ping",
        description: "Check if EZCAP IPC is responsive.",
        inputSchema: { type: "object", properties: {} },
      },
      {
        name: "app.info",
        description: "Get EZCAP build/version information.",
        inputSchema: { type: "object", properties: {} },
      },
      {
        name: "camera.status",
        description: "Get current camera status from EZCAP.",
        inputSchema: { type: "object", properties: {} },
      },
      {
        name: "help.methods",
        description: "List supported EZCAP IPC methods.",
        inputSchema: { type: "object", properties: {} },
      },
    ],
  };
}

app.get("/healthz", (_req, res) => {
  res.status(200).json({ status: "ok" });
});

app.post(PATH, async (req, res) => {
  if (!isAuthorized(req)) {
    res.status(401).json(jsonRpcError(req.body?.id, 401, "Unauthorized"));
    return;
  }

  const { jsonrpc, id, method, params } = req.body || {};
  if (jsonrpc !== "2.0" || !method) {
    res.status(400).json(jsonRpcError(id, -32600, "Invalid Request"));
    return;
  }

  // Notifications: no response.
  const isNotification = typeof id === "undefined";

  try {
    if (method === "initialize") {
      const result = {
        protocolVersion: "2024-11-05",
        capabilities: { tools: {} },
        serverInfo: {
          name: "ezcap-mcp-server",
          version: "0.1.0",
        },
      };
      if (isNotification) {
        res.status(204).end();
      } else {
        res.json({ jsonrpc: "2.0", id, result });
      }
      return;
    }

    if (method === "notifications/initialized") {
      res.status(204).end();
      return;
    }

    if (method === "tools/list") {
      const result = toolsListResult();
      if (isNotification) {
        res.status(204).end();
      } else {
        res.json({ jsonrpc: "2.0", id, result });
      }
      return;
    }

    if (method === "tools/call") {
      const toolName = params?.name;
      const toolArgs = params?.arguments || {};
      if (!toolName) {
        res.status(400).json(jsonRpcError(id, -32602, "Missing tool name"));
        return;
      }
      const result = await callIpc(toolName, toolArgs);
      const content = [{ type: "text", text: JSON.stringify(result, null, 2) }];
      res.json({ jsonrpc: "2.0", id, result: { content } });
      return;
    }

    res.status(404).json(jsonRpcError(id, -32601, "Method not found"));
  } catch (error) {
    res.status(500).json(jsonRpcError(id, -32000, error.message || "Server error"));
  }
});

app.listen(PORT, () => {
  console.log(`EZCAP MCP HTTP server listening on http://127.0.0.1:${PORT}${PATH}`);
});
