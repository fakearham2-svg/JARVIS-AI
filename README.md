# JARVIS AI

A mobile-friendly AI assistant web app powered by NVIDIA's API, with optional ElevenLabs voice output.

## Features

- Chat interface with persistent local chat history and projects
- NVIDIA model discovery, manual model selection, and automatic model routing
- Streaming AI responses
- Optional web search endpoint
- Image and document attachments (including PDF text extraction)
- HTML/code preview with mobile and desktop viewport controls
- Screenshot-aware debugging prompts
- Voice mode with browser speech recognition and optional ElevenLabs voices/text-to-speech
- Theme and chat font settings
- Export/import chat history and a local memory vault

## Requirements

- [Bun](https://bun.sh/) 1.1 or newer
- An NVIDIA API key from [NVIDIA API Catalog](https://build.nvidia.com/)
- Optional ElevenLabs API key for ElevenLabs voice output

## Run locally

```bash
bun install
bun run index.ts
```

Open http://localhost:3000.

## Configure API keys

Open JARVIS Settings in the browser and enter your NVIDIA API key. The key is kept in that browser's local storage and sent to the server for the relevant request; it is not committed to this repository or stored as a Railway variable.

ElevenLabs is optional. Add its API key in Voice Settings to load voices and use ElevenLabs text-to-speech. Without it, the app falls back to browser speech synthesis where supported.

## Deploy on Railway

1. Create a Railway project and choose **Deploy from GitHub repo**.
2. Select `fakearham2-svg/JARVIS-AI`.
3. Railway reads `railway.json` and starts the app with `bun run index.ts`.
4. Generate a public domain for the service.

The app exposes `/health` for the Railway health check.

## Notes

- Chat history, projects, preferences, and user-entered API keys are browser-local; they do not automatically sync between devices.
- Do not commit API keys, access tokens, or private credentials.
- This repository contains the current application source transferred from the Railway production function. The Railway service has not been removed by this transfer.
