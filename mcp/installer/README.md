# MCP Installer Setup

## Overview
MCP Installer is a server that helps install other MCP servers. It provides a simple way to install MCP servers hosted in npm or PyPi through Claude.

## Prerequisites
- `npx` for Node.js based servers
- `uv` for Python based servers

## Installation

1. Add the following configuration to your `claude_desktop_config.json`:
   - For macOS: Located at `~/Library/Application Support/Claude`
   - For Windows: Located at `C:\Users\NAME\AppData\Roaming\Claude`

```json
{
  "mcpServers": {
    "mcp-installer": {
      "command": "npx",
      "args": [
        "@anaisbetts/mcp-installer"
      ]
    }
  }
}
```

## Usage Examples

You can ask Claude to perform installations using natural language:

1. Install a specific MCP server:
```
Hey Claude, install the MCP server named mcp-server-fetch
```

2. Install with custom arguments:
```
Hey Claude, install the @modelcontextprotocol/server-filesystem package as an MCP server. Use ['/path/to/directory'] for the arguments
```

3. Install from local path:
```
Hi Claude, please install the MCP server at /path/to/local/server
```

4. Install with environment variables:
```
Install the server @modelcontextprotocol/server-github. Set the environment variable GITHUB_PERSONAL_ACCESS_TOKEN to 'your-token'
```

## Troubleshooting

1. Ensure npx is installed and accessible from command line
2. Verify the configuration file is in the correct location
3. Check that the file permissions are correct
4. Ensure you have necessary environment variables set

## References
- Original repository: [anaisbetts/mcp-installer](https://github.com/anaisbetts/mcp-installer)
- Package on npm: [@anaisbetts/mcp-installer](https://www.npmjs.com/package/@anaisbetts/mcp-installer)