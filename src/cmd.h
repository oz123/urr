/*
 * urr - A lightweight Wake-on-LAN tool
 * Copyright (C) 2025  Oz Tiram <oz.tiram@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef CMD_H
#define CMD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define CMD_VERSION 1
#define CMD_HELP    2
#define CMD_FILE    3

#define PROGNAME "urr"
#ifndef VERSION
#define VERSION "0.1"
#endif
#define AUTHOR "Oz Tiram"

static struct command {
    const char    *name;
    const char    *shortcut;
    const char    *descr;
    const int      cmdtype;
} cmd[] = {
    { "--version", "-v", "Show the program version", CMD_VERSION},
    { "--help",    "-h", "Show this help", CMD_HELP},
    { "--file",    "-f", "File containing MAC addresses to wake", CMD_FILE},
    { NULL, NULL, NULL, -1}
};

static int find_cmd(const char *arg) {
    for (int i = 0; cmd[i].name != NULL; i++) {
        if (strcmp(arg, cmd[i].name) == 0 || strcmp(arg, cmd[i].shortcut) == 0)
            return cmd[i].cmdtype;
    }
    return -1;
}

void help(const struct command *cmd) {
    fprintf(stderr, "Usage: " PROGNAME " [options] <MAC_ADDRESS | target_name>\n");
    fprintf(stderr, "\nOptions:\n");
    for (int i = 0; cmd[i].name != NULL; i++) {
         fprintf(stderr, "  %s, %s\t\t%s\n", cmd[i].shortcut, cmd[i].name, cmd[i].descr);
    }
}

void version(void) {
    fprintf(stderr, PROGNAME " " VERSION "\n");
    fprintf(stderr, "Copyright (C) 2025 " AUTHOR "\n");
    fprintf(stderr, "License: GPL-3.0-or-later <https://gnu.org/licenses/gpl.html>\n");
    fprintf(stderr, "This is free software: you are free to change and redistribute it.\n");
    fprintf(stderr, "There is NO WARRANTY, to the extent permitted by law.\n");
}

void parse_args(int argc, char *argv[], char **filename, char **lookup) {
    *filename = NULL;
    *lookup = NULL;
    static char default_path[PATH_MAX]; // Static so it persists after function returns

    if (argc < 2) {
        help(cmd);
        exit(1);
    }

    for (int i = 1; i < argc; i++) {
        int ch = find_cmd(argv[i]);
        switch (ch) {
            case CMD_HELP:
                help(cmd);
                exit(0);
            case CMD_VERSION:
                version();
                exit(0);
            case CMD_FILE:
                if (i + 1 < argc) {
                    *filename = argv[++i]; // Set filename and skip next arg
                } else {
                    fprintf(stderr, "Error: -f requires a filename\n");
                    exit(1);
                }
                break;
            default:
                // If it's not a known flag, it must be our target (MAC or Hostname)
                *lookup = argv[i];
                break;
        }
    }

    // If no file provided, build the ~/.config path
    if (*filename == NULL) {
        const char *home = getenv("HOME");
        if (home == NULL) {
            // Fallback to local file if HOME isn't set
            *filename = "hosts";
        } else {
            snprintf(default_path, sizeof(default_path), "%s/.config/urr/hosts", home);
            *filename = default_path;
        }
    }
}

#endif
