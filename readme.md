# Task Management App (Server + GTK UI)

This guide only covers how to run the server and the GTK UI.

## Prerequisites

- Ubuntu/Debian with:
  - `g++`, `make`
  - `libpq-dev`, `libssl-dev`
  - `gtk+-3.0` dev packages
  - `nlohmann-json3-dev`
- PostgreSQL database configured for this project

## Run (2 terminals)

### Terminal 1 — build + run server

```bash
cd /home/shuni/ProjectNetPro/HUST-NetworkProgramming-TaskManagement

make

./build/server
```

Expected output:
```
===== Task Management Server =====
Database connected successfully
Server listening on port 8080
```

### Terminal 2 — build + run GTK UI

```bash
cd UI

make -f Makefile.integrated

env -i HOME=$HOME DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY PATH=/usr/bin:/bin ./build/start_integrated
```

Note:
- The UI connects to `127.0.0.1:8080`.
- OTP codes appear in the **server terminal**.

## If UI fails to start (Snap libpthread error)

Always run the UI with the `env -i ...` command above to avoid Snap library conflicts.

