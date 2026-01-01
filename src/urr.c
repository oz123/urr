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
#include <netinet/ether.h>
#include <unistd.h>
#include <regex.h>
#include <strings.h>
#include <sys/socket.h>

/**
 * Validates a hostname and looks up the MAC from /etc/ethers format.
 * /etc/ethers format: "mac_address hostname"
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
        // Skip comments or lines that are too short to be "mac host"
        if (line[0] == '#' || line[0] == '\n' || strlen(line) < 20) continue;

        // /etc/ethers format: MAC first, then hostname
        // %17s prevents overflow of 18-byte mac buffer
        // %255s prevents overflow of 256-byte host buffer
        if (sscanf(line, "%17s %255s", file_mac, file_host) == 2) {
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
    struct ether_addr *ea = ether_aton(mac_str);

    // If first attempt fails and string contains dashes, try converting
    if (ea == NULL && strchr(mac_str, '-')) {
        char *normalized_mac = strdup(mac_str);
        if (!normalized_mac) return -1;

        for (char *p = normalized_mac; *p; p++) {
            if (*p == '-') *p = ':';
        }

        ea = ether_aton(normalized_mac);
        free(normalized_mac);
    }

    if (ea == NULL) {
        return -1;
    }

    memcpy(mac_out, ea->ether_addr_octet, 6);
    return 0;
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

int main(const int argc, char *argv[]) {
    char *filename = NULL;
    char *input_arg = NULL;
    unsigned char bin_mac[6];
    char resolved_mac[18]; // Exactly the size for a MAC + \0

    parse_args(argc, argv, &filename, &input_arg);

    if (!input_arg) {
        fprintf(stderr, "Error: No target specified.\n");
        return 1;
    }

    // 1. First Attempt: Try to validate and parse as MAC address
    if (validate_and_parse_mac(input_arg, bin_mac) == 0) {
        // Valid MAC address - send directly
        send_wol(bin_mac);
        printf("Direct MAC detected. Magic packet sent to %s\n", input_arg);
    } else {
        // 2. Not a valid MAC - treat as hostname and search files
        int found = 0;
        char *source_file = NULL;

        // Try user-specified file first (if provided)
        if (filename && lookup_mac_in_file(filename, input_arg, resolved_mac) == 0) {
            found = 1;
            source_file = filename;
        }
        // If not found in user file, try /etc/ethers
        else if (lookup_mac_in_file("/etc/ethers", input_arg, resolved_mac) == 0) {
            found = 1;
            source_file = "/etc/ethers";
        }

        if (found) {
            // Validate the MAC we found in the file
            if (validate_and_parse_mac(resolved_mac, bin_mac) == 0) {
                send_wol(bin_mac);
                printf("Resolved host '%s' to %s in %s. Packet sent.\n",
                        input_arg, resolved_mac, source_file);
            } else {
                fprintf(stderr, "Error: MAC '%s' in file %s is invalid.\n",
                        resolved_mac, source_file);
                return 1;
            }
        } else {
            // Build error message showing which files were checked
            if (filename) {
                fprintf(stderr, "Error: '%s' is not a valid MAC and not found in %s or /etc/ethers\n",
                        input_arg, filename);
            } else {
                fprintf(stderr, "Error: '%s' is not a valid MAC and not found in /etc/ethers\n",
                        input_arg);
            }
            return 1;
        }
    }

    return 0;
}