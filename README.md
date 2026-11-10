# Concurrent Client-Server System in C

## Overview

A concurrent client-server application implemented in C as part of an Operating Systems assignment.

The system implements a server that performs arithmetic operations requested by multiple client processes. Inter-process communication is achieved using Linux signals and file-based data exchange, without network sockets.

## Features

- **Signal-Based IPC:** Uses `SIGUSR1` and `SIGUSR2` for asynchronous communication between client and server processes.
- **File-Based Communication:** Exchanges requests and responses through shared and client-specific files.
- **Concurrent Processing:** Uses `fork()` to create a dedicated child process for each client request.
- **Process Management:** Separates request handling from the main server process.
- **Timeout Handling:** Uses `alarm()` to implement client response and server inactivity timeouts.
- **Arithmetic Operations:** Supports addition, subtraction, multiplication, and division.
- **Error Handling:** Includes system call validation and temporary file management.

## Supported Operations

| Operation | Code |
|---|---|
| Addition (+) | 1 |
| Subtraction (-) | 2 |
| Multiplication (*) | 3 |
| Division (/) | 4 |

## Technologies

- C (C99)
- Linux / POSIX System Programming
- Process Management (`fork()`)
- Inter-Process Communication (IPC)
- Linux Signals (`SIGUSR1`, `SIGUSR2`)
- File I/O
- Timers (`alarm()`)
- CMake

## Project Structure

```text
concurrent-client-server-c/
├── CMakeLists.txt
├── server.c
├── client.c
├── README.md
└── .gitignore
```

## Build Instructions

### Requirements
- Linux environment
- GCC
- CMake 3.17 or later

### Build

```bash
cmake -S . -B build
cmake --build build
```

## Usage

### 1. Start the server

```bash
./build/server &
```

### 2. Find the server PID

```bash
ps
```

### 3. Send a calculation request

```bash
./build/client <SERVER_PID> <NUM1> <OPERATION_CODE> <NUM2>
```

Example:

```bash
./build/client 1234 70 2 30
```

Expected calculation:

```text
70 - 30 = 40
```

The client receives the result from the server and prints it to the terminal.

Multiple client processes can send independent requests to the server.

## Communication Architecture

The system combines signal-based notifications with file-based data exchange:

- `to_srv`: Shared file used to transfer client PID and calculation parameters to the server.
- `to_client_<PID>`: Client-specific file used to transfer calculation results from the server.

When a request arrives, the server creates a child process using `fork()`. The child processes the request, writes the result to the corresponding response file, and notifies the client using a signal.

The parent server process continues listening for additional requests.

---
