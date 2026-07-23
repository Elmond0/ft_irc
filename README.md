*This project has been created as part of the 42 curriculum by [elmondo, miricci, giomastr]*

# ft_irc

## Description

`ft_irc` is a project from the 42 curriculum whose goal is to implement a functional **IRC (Internet Relay Chat) server** from scratch, written in C++98, without using any external IRC library.

The server is designed to:
- Handle **multiple clients simultaneously**, without ever forking a new process for each connection (using a single-threaded, non-blocking I/O model with `poll()` — or `select()`/`epoll()`/`kqueue()`).
- Accept connections from a real IRC client (such as **irssi**, **HexChat**, **WeeChat**, or a custom TCP client like `nc`) using the standard IRC protocol.
- Implement the core mechanisms of a real IRC server: client authentication, nicknames, channels, and communication between clients.

The purpose of this project is to understand and apply key network programming concepts: non-blocking sockets, the `poll()` (or equivalent) system call, TCP communication, and the design of a text-based application-layer protocol.

### Main features implemented

- Client connection and authentication (`PASS`, `NICK`, `USER`)
- Private messages between clients (`PRIVMSG`)
- Channel management:
  - `JOIN` — join a channel
  - `PART` — leave a channel
  - `TOPIC` — get/set the channel topic
  - `KICK` — eject a client from a channel
  - `INVITE` — invite a client to a channel
  - `MODE` — channel operator privileges and modes:
    - `i`: invite-only channel
    - `t`: topic settable by operator only
    - `k`: channel password
    - `o`: operator privilege
    - `l`: user limit on the channel
- Operator/regular user distinction within channels

## Instructions

### Requirements

- A Unix-like environment (Linux / macOS)
- A C++ compiler supporting **C++98**
- `make`

### Compilation

Clone the repository and build the project with `make` at the root of the repository:

```bash
git clone <repository_url>
cd ft_irc
make
```

This generates the `ircserv` executable. Other available Makefile rules:

```bash
make clean   # remove object files
make fclean  # remove object files and the executable
make re      # fclean + make (full rebuild)
```

### Execution

The server is launched as follows:

```bash
./ircserv <port> <password>
```

- `<port>` — the port number on which the server will listen for incoming connections.
- `<password>` — the connection password that clients must provide to authenticate.

Example:

```bash
./ircserv 6667 mypassword
```

### Connecting a client

You can connect to the server using any standard IRC client, for example:

```bash
irssi -c 127.0.0.1 -p 6667 -w mypassword
```

Or, for basic manual testing, using `nc`:

```bash
nc 127.0.0.1 6667
```

and then typing IRC commands manually (e.g. `PASS mypassword`, `NICK myname`, `USER myuser 0 * :Real Name`).

## Resources

### Documentation and references

- [RFC 1459 — Internet Relay Chat Protocol](https://datatracker.ietf.org/doc/html/rfc1459)
- [RFC 2812 — Internet Relay Chat: Client Protocol](https://datatracker.ietf.org/doc/html/rfc2812)
- [Modern IRC Client Protocol documentation (modern.ircdocs.horse)](https://modern.ircdocs.horse/)
- `man poll`, `man select`, `man socket`, `man bind`, `man listen`, `man accept`, `man recv`, `man send`
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)

### AI usage

AI tools (e.g. ChatGPT / Claude) were used during the development of this project for the following purposes:
- Understanding new topics and how to optimise them within the code.
- Creating flow diagrams and schemes to visualise the processing of messages to better design the project.
- Clarifying ambiguous parts of the RFC 1459 / 2812 specifications regarding IRC command syntax and expected server replies.
- Assisting in debugging socket-related issues (e.g. handling partial reads/writes and non-blocking I/O edge cases).
- Reviewing code structure, suggesting safety fixes and checking for adherence to C++98 constraints.

AI tools were **not** used to generate core business logic (command parsing, channel/client management, protocol state machine) without review; all AI-assisted output was manually reviewed, tested, and adapted by the author(s) before being integrated into the project.
