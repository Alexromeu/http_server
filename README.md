# HTTP Server (C)

A minimal HTTP server implemented in C using raw Berkeley sockets.  
This project is an educational exploration of how HTTP works at the protocol level and how to build a server from scratch without external frameworks.

---

## Features

- Basic TCP server using `socket()`, `bind()`, `listen()`, and `accept()`
- Reads raw HTTP requests from clients
- Sends minimal valid HTTP responses
- Includes a simple client for testing request/response behavior
- Clean, small codebase intended for incremental expansion

---

## Project Structure


---

## Build

Compile both programs with GCC:

```sh
gcc server.c -o server
gcc client.c -o client
