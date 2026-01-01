# Urr (עוּר)

`urr` is a lightweight, dependency-free Wake-on-LAN (WoL)
utility written in C.

Unlike simple WoL tools, urr allows you to manage a
database of machines, letting you wake them up using 
friendly hostnames instead of memorizing 17-character
MAC addresses.

The name urr comes from the Hebrew imperative עוּר, meaning "wake up!" or "arise!".

## Features

 * Fast & Tiny: Written in pure C with no external library dependencies (uses standard POSIX headers).
 * Dual Mode: Accepts direct MAC addresses or hostnames.
 * Configurable: Uses ~/.config/urr/hosts for easy machine management.
 * Safe: Implements strict Regex validation for MAC addresses and protects against buffer overflows.
 * License: GPL-3.0-or-later license.

## Installation Prerequisites

 * A C compiler (GCC or Clang)
 * make
 * scdoc (optional, for generating the man page)

### building
```
$ git clone https://github.com/oznt/urr
$ cd urr
$ make
```

### system wide install
```
$ sudo make install
```

## Usage

1. Waking by MAC Address

You can use colons or hyphens:
```
$ urr 00:11:22:33:44:55
$ urr AA-BB-CC-DD-EE-FF
```

2. Waking by Hostname

First, add your machines to ~/.config/urr/hosts:

```plaintext
# hostname   mac-address
00:11:22:33:44:55 nas
AA:BB:CC:DD:EE:FF gaming-rig
```

Then simply run:
```
$ urr nas
```

Also, /etc/ethers is supported too. See `man 5 ethers`.


3. Using a Custom File

```
$ urr -f my_servers.txt web-server
```

## Development & Testing

The project includes a functional test suite to ensure regex and file parsing logic remains solid.

```
$ make test
```

### License

Copyright © 2025 Oz Tiram <oz.tiram@gmail.com>
This project is licensed under the GNU General Public License v3.0 or later.
See the COPYING file for details.
