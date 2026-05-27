import net from "net";

const PIPE_NAME =
  process.env.EZCAP_MCP_PIPE ||
  (process.platform === "win32" ? "\\\\.\\pipe\\ezcap_mcp" : "/tmp/ezcap_mcp");

const REQUEST_TIMEOUT_MS = Number.parseInt(
  process.env.EZCAP_MCP_TIMEOUT_MS || "2000",
  10
);

let nextId = 1;

export function callIpc(method, params = {}) {
  return new Promise((resolve, reject) => {
    const socket = net.connect(PIPE_NAME);
    const id = nextId++;
    let buffer = "";
    let settled = false;

    const cleanup = () => {
      if (!socket.destroyed) {
        socket.end();
      }
    };

    const timeout = setTimeout(() => {
      if (settled) {
        return;
      }
      settled = true;
      cleanup();
      reject(new Error(`IPC timeout after ${REQUEST_TIMEOUT_MS}ms`));
    }, REQUEST_TIMEOUT_MS);

    socket.on("connect", () => {
      const payload = JSON.stringify({
        jsonrpc: "2.0",
        id,
        method,
        params,
      });
      socket.write(`${payload}\n`);
    });

    socket.on("data", (chunk) => {
      buffer += chunk.toString("utf8");
      let idx = buffer.indexOf("\n");
      while (idx >= 0) {
        const line = buffer.slice(0, idx).trim();
        buffer = buffer.slice(idx + 1);
        if (line.length > 0) {
          try {
            const message = JSON.parse(line);
            if (message.id !== id) {
              idx = buffer.indexOf("\n");
              continue;
            }
            if (message.error) {
              const err = new Error(message.error.message || "IPC error");
              err.code = message.error.code;
              throw err;
            }
            if (!settled) {
              settled = true;
              clearTimeout(timeout);
              cleanup();
              resolve(message.result);
            }
          } catch (error) {
            if (!settled) {
              settled = true;
              clearTimeout(timeout);
              cleanup();
              reject(error);
            }
          }
        }
        idx = buffer.indexOf("\n");
      }
    });

    socket.on("error", (error) => {
      if (!settled) {
        settled = true;
        clearTimeout(timeout);
        cleanup();
        reject(error);
      }
    });

    socket.on("close", () => {
      if (!settled) {
        settled = true;
        clearTimeout(timeout);
        reject(new Error("IPC connection closed"));
      }
    });
  });
}
