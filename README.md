*This project has been created as part of the 42 curriculum by elmondo, miricci, giomastr*

# ft_irc

## Description

`ft_irc` is a C++98 IRC server built from scratch as part of the 42 curriculum.
It handles multiple clients simultaneously using a single non-blocking `poll()` loop — no threads, no forks.

Reference client: **HexChat**

### Features

- Client authentication (`PASS`, `NICK`, `USER`)
- Private messages (`PRIVMSG`)
- Channel management with operator/regular user distinction
- Operator commands: `KICK`, `INVITE`, `TOPIC`, `MODE`
- Channel modes: `i` (invite-only), `t` (topic restricted), `k` (password), `o` (operator), `l` (user limit)
- IRC bot (`!help`, `!users`, `!topic`)

## Instructions

### Compilation

```bash
make
```

Available rules: `make clean`, `make fclean`, `make re`

### Usage

```bash
./ircserv <port> <password>
```

Example:

```bash
./ircserv 6667 mypassword
```

### Connect with HexChat

1. Open HexChat → Network list → Add new network
2. Server: ip-server / port number (`127.0.0.1/6667`)
3. Set the server password
4. Connect

### Connect with nc (manual testing)

```bash
nc -C localhost 6667
```

## Commands

### Registration

```
PASS <password>
NICK <nickname>
USER <username> :<realname>
```

### Channels

```
JOIN #<channel> [password]
PART #<channel> [reason]
PRIVMSG #<channel> :<message>
PRIVMSG <nickname> :<message>
TOPIC #<channel> [new topic]
```

### Operator commands

```
KICK #<channel> <nickname> [reason]
INVITE <nickname> #<channel>
MODE #<channel> +i              # invite-only on
MODE #<channel> -i              # invite-only off
MODE #<channel> +t              # topic restricted to operators
MODE #<channel> +k <password>   # set channel password
MODE #<channel> +o <nickname>   # give operator privilege
MODE #<channel> +l <limit>      # set user limit
```

### Other

```
QUIT [reason]
PING <token>
```

### Bot commands (send in channel or DM to ircbot)

```
!help    list available commands
!users   connected users and channel members
!topic   show the channel topic
```

## Resources

- [RFC 1459 — IRC Protocol](https://datatracker.ietf.org/doc/html/rfc1459)
- [RFC 2812 — IRC Client Protocol](https://datatracker.ietf.org/doc/html/rfc2812)
- [Modern IRC documentation](https://modern.ircdocs.horse/)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- `man poll`, `man socket`, `man recv`, `man send`

### AI usage

AI tools (Claude, ChatGPT) were used to:

- Clarify ambiguous parts of RFC 1459/2812
- Clarify non-blocking I/O edge cases

All AI-assisted output was reviewed, tested, and adapted by the authors before integration.
