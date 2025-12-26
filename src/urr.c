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


#include "cmd.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <regex.h>
#include <strings.h>
#include <sys/socket.h>

#define MAC_REGEX "^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$"


/**
 * Validates a hostname and looks up the MAC.
 * file_host buffer: 256 (RFC 1035 max is 255 + \0)
 * file_mac buffer: 18 (17 chars + \0)
 */
int lookup_mac_in_file(const char *filename, const char *search_host, char *found_mac) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return -1;

    // Buffers sized to exact protocol maximums
    char line[512]; 
    char file_host[256]; 
    char file_mac[18];   
    int found = 0;

    while (fgets(line, sizeof(line), fp)) {
        // Skip comments or lines that are too short to be "host mac"
        if (line[0] == '#' || line[0] == '\n' || strlen(line) < 20) continue;

        // %255s prevents overflow of 256-byte host buffer
        // %17s prevents overflow of 18-byte mac buffer
        if (sscanf(line, "%255s %17s", file_host, file_mac) == 2) {
            if (strcasecmp(file_host, search_host) == 0) {
                // Ensure found_mac (passed from main) is at least 18 bytes
                strncpy(found_mac, file_mac, 18);
                found = 1;
                break;
            }
        }
    }
    fclose(fp);
    return found ? 0 : -1;
}

int validate_and_parse_mac(const char *mac_str, unsigned char *mac_out) {
    regex_t regex;
    int reti;

    // 1. Compile the regular expression
    reti = regcomp(&regex, MAC_REGEX, REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        return -1;
    }

    // 2. Execute regex match
    reti = regexec(&regex, mac_str, 0, NULL, 0);
    regfree(&regex); // Free memory allocated by regcomp

    if (reti == REG_NOMATCH) {
        return -1; // Not a valid MAC format
    } else if (reti != 0) {
        return -1; // Some other error
    }

    // 3. If valid, parse into bytes
    // We can use sscanf now knowing the format is safe
    unsigned int m[6];
    // This format handles both : and - because sscanf skips the literal separator
    if (sscanf(mac_str, "%x%*c%x%*c%x%*c%x%*c%x%*c%x", 
               &m[0], &m[1], &m[2], &m[3], &m[4], &m[5]) == 6) {
        for (int i = 0; i < 6; i++) mac_out[i] = (unsigned char)m[i];
        return 0;
    }

    return -1;
}

void send_wol(unsigned char *mac) {
    unsigned char packet[102];
    memset(packet, 0xFF, 6);
    for (int i = 0; i < 16; i++) memcpy(packet + 6 + (i * 6), mac, 6);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    int broadcast = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(9), 
                                .sin_addr.s_addr = inet_addr("255.255.255.255") };

    sendto(sock, packet, sizeof(packet), 0, (struct sockaddr *)&addr, sizeof(addr));
    close(sock);
}

int main(int argc, char *argv[]) {
    char *filename = NULL;
    char *input_arg = NULL;
    unsigned char bin_mac[6];
    char resolved_mac[18]; // Exactly the size for a MAC + \0

    parse_args(argc, argv, &filename, &input_arg);

    if (!input_arg) {
        fprintf(stderr, "Error: No target specified.\n");
        return 1;
    }

    // 1. First Attempt: Is the argument itself a valid MAC?
    if (validate_and_parse_mac(input_arg, bin_mac) == 0) {
        send_wol(bin_mac);
        printf("Direct MAC detected. Magic packet sent to %s\n", input_arg);
    }
    // 2. Second Attempt: Treat it as a hostname and look in the file
    else {
        if (lookup_mac_in_file(filename, input_arg, resolved_mac) == 0) {
            // Validate the MAC we found in the file
            if (validate_and_parse_mac(resolved_mac, bin_mac) == 0) {
                send_wol(bin_mac);
                printf("Resolved host '%s' to %s in %s. Packet sent.\n",
                        input_arg, resolved_mac, filename);
            } else {
                fprintf(stderr, "Error: MAC '%s' in file %s is invalid.\n",
                        resolved_mac, filename);
                return 1;
            }
        } else {
            fprintf(stderr, "Error: '%s' is not a valid MAC and not found in %s\n",
                    input_arg, filename);
            return 1;
        }
    }

    return 0;
}
