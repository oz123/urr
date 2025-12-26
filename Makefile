###
SHELL := /bin/bash
.DEFAULT_GOAL := help

.PHONY: help
help:
	@mh -f $(MAKEFILE_LIST) $(target) || echo "Please install mh from https://github.com/oz123/mh/releases"
ifndef target
	@(which mh > /dev/null 2>&1 && echo -e "\nUse \`make help target=foo\` to learn more about foo.")
endif

# Variables
CC      = gcc
CFLAGS  = -Wall -Wextra -O2
TARGET  = urr
TEST_TARGET = run_tests
SRC     = src/urr.c
PREFIX  = /usr/local
BINDIR  = $(PREFIX)/bin
MANDIR  = $(PREFIX)/share/man/man1
SHAREDIR = $(PREFIX)/share/urr/examples

all: $(TARGET)  ## default build target

$(TARGET): $(SRC) ## create the final executable
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

debug: CFLAGS = -Wall -Wextra -g
debug: all  ## debug build with symbols for GDB

$(TEST_TARGET): test.c  ## compile the test runner
	$(CC) $(CFLAGS) -o $(TEST_TARGET) test.c

test: $(TARGET) $(TEST_TARGET)  ## run the tests
	./$(TEST_TARGET)
	rm -f $(TEST_TARGET)

man:  ## create the man page
	scdoc < docs/$(TARGET).scd > docs/$(TARGET).1
	@echo "Man page generated at docs/$(TARGET).1"

clean:  ## clean up the build files
	rm -f $(TARGET) $(TEST_TARGET) docs/$(TARGET).1

install: $(TARGET) man  ## install the program globally
	@echo "Installing binary to $(BINDIR)..."
	install -d $(BINDIR)
	install -m 0755 $(TARGET) $(BINDIR)
	@echo "Installing man page to $(MANDIR)..."
	install -d $(MANDIR)	
	install -m 0644 docs/$(TARGET).1 $(MANDIR)
	@echo "Installing example hosts file to $(SHAREDIR)..."
	install -d $(SHAREDIR)
	@echo "# Format: hostname mac-address" > hosts.example
	@echo "# Example: nas 00:11:22:33:44:55" >> hosts.example
	install -m 0644 hosts.example $(SHAREDIR)/hosts
	@rm hosts.example

uninstall:  # remove the program from the system
	@echo "Removing binary from $(BINDIR)..."
	rm -f $(BINDIR)/$(TARGET)
	rm -f $(MANDIR)/$(TARGET).1
	@echo "Note: Configuration in $(CONFDIR) was left intact. Delete it manually if desired."


LDFLAGS_STATIC = -static
static: $(SRC)  ## build a static version
	$(CC) $(CFLAGS) $(LDFLAGS_STATIC) -o $(TARGET) $(SRC)
	strip $(TARGET)
	@echo "Static and stripped binary created: $(TARGET)"

PHONY: all debug clean test install uninstall
