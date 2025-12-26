#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BINARY "./urr"
#define TEST_DIR "/tmp/urr_test"
#define TEST_HOSTS "/tmp/urr_test/hosts"

void setup_test_env() {
    mkdir(TEST_DIR, 0700);
    FILE *fp = fopen(TEST_HOSTS, "w");
    if (fp) {
        fprintf(fp, "nas 00:11:22:33:44:55\n");
        fprintf(fp, "workpc AA:BB:CC:DD:EE:FF\n");
        fprintf(fp, "rpi 11-22-CC-DD-EE-FF\n");
        fprintf(fp, "# comment_line 11:22:33:44:55:66\n");
        fclose(fp);
    }
}

void run_test(const char *description, const char *args, int expected_exit) {
    char command[512];
    printf("[TEST] %-40s ", description);
    
    // Construct command: Pass filename explicitly or mock HOME for default path tests
    snprintf(command, sizeof(command), "%s %s > /dev/null 2>&1", BINARY, args);
    
    int result = system(command);
    int exit_status = WEXITSTATUS(result);

    if (exit_status == expected_exit) {
        printf("\033[0;32mPASS\033[0m\n");
    } else {
        printf("\033[0;31mFAIL (Expected %d, got %d)\033[0m\n", expected_exit, exit_status);
    }
}

int main() {
    setup_test_env();

    printf("Starting Urr Functional Tests...\n");
    printf("------------------------------------\n");

    // Test Case 1: Valid Direct MAC
    run_test("Direct MAC (Colons)", "00:1A:2B:3C:4D:5E", 0);

    // Test Case 2: Valid Direct MAC (Hyphens)
    run_test("Direct MAC (Hyphens)", "00-1A-2B-3C-4D-5E", 0);

    // Test Case 3: Invalid MAC format
    run_test("Invalid MAC format", "00:11:22:33:44:GG", 1);

    // Test Case 4: File Lookup (Explicit file)
    char file_arg[256];
    snprintf(file_arg, sizeof(file_arg), "-f %s nas", TEST_HOSTS);
    run_test("File lookup (hostname 'nas')", file_arg, 0);
    run_test("File lookup (hostname 'rpi')", file_arg, 0);

    // Test Case 5: Hostname not in file
    snprintf(file_arg, sizeof(file_arg), "-f %s non-existent", TEST_HOSTS);
    run_test("File lookup (missing host)", file_arg, 1);

    // Test Case 6: No arguments
    run_test("No arguments", "", 1);

    printf("------------------------------------\n");
    printf("Tests complete. Cleaning up...\n");
    unlink(TEST_HOSTS);
    rmdir(TEST_DIR);

    return 0;
}
