import { Server } from "@modelcontextprotocol/sdk/server/index.js";
import { StdioServerTransport } from "@modelcontextprotocol/sdk/server/stdio.js";
import { callIpc } from "./ipcClient.js";

const server = new Server(
  {
    name: "ezcap-mcp-server",
    version: "0.1.0",
  },
  {
    capabilities: {
      tools: {},
    },
  }
);

server.setRequestHandler("tools/list", async () => {
  return {
    tools: [
      {
        name: "app.ping",
        description: "Check if EZCAP IPC is responsive.",
        inputSchema: {
          type: "object",
          properties: {},
        },
      },
      {
        name: "app.info",
        description: "Get EZCAP build/version information.",
        inputSchema: {
          type: "object",
          properties: {},
        },
      },
      {
        name: "camera.status",
        description: "Get current camera status from EZCAP.",
        inputSchema: {
          type: "object",
          properties: {},
        },
      },
      {
        name: "help.methods",
        description: "List supported EZCAP IPC methods.",
        inputSchema: {
          type: "object",
          properties: {},
        },
      },
    ],
  };
});

server.setRequestHandler("tools/call", async (request) => {
  const { name, arguments: args } = request.params ?? {};
  if (!name) {
    return {
      content: [{ type: "text", text: "Missing tool name." }],
    };
  }

  const result = await callIpc(name, args || {});
  return {
    content: [
      {
        type: "text",
        text: JSON.stringify(result, null, 2),
      },
    ],
  };
});

const transport = new StdioServerTransport();
await server.connect(transport);
