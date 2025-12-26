urr(1) "December 2025" "Wake-on-LAN Utility"

# NAME

urr - A lightweight Wake-on-LAN tool with hostname resolution

# SYNOPSIS

**urr** [*-f* _FILE_] [_TARGET_]
**urr** [*-h* | *--help*]
**urr** [*-v* | *--version*]

# DESCRIPTION

**urr** sends a "Magic Packet" to a target network card to wake a computer over
the network. The program first checks if the *TARGET* is a valid MAC address.
If it is not, it attempts to resolve the *TARGET* as a hostname by looking it
up in a configuration file.
Urr is imperative: "wake up!" in Hebrew.

# OPTIONS

**-f**, **--file** _FILE_
	Use _FILE_ as the hosts database instead of the default.

**-h**, **--help**
	Show the help message and exit.

**-v**, **--version**
	Show program version and license information.

# FILES

*~/.config/urr/hosts*
	The default database file. The format is "hostname mac-address" per line.
	Lines starting with '#' are ignored.

*/usr/local/share/urr/examples/hosts*
	A template file provided during installation.

# EXAMPLES

**urr 00:11:22:33:44:55**
	Send a magic packet to a specific MAC address.

**urr nas**
	Lookup the MAC address for 'nas' in the default hosts file and wake it.

**urr -f ./my_hosts office-pc**
	Use a local file to resolve 'office-pc'.

# AUTHORS

Oz Tiram <oz.tiram@gmail.com>

# BUGS

See GitHub issues at <https://github.com/oz123/urr>

# LICENSE

GPL-3.0-or-later
