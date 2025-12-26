# Makefile for uf2conv - UF2 format converter
# Builds native binary for current platform

CC ?= gcc
CFLAGS = -O2 -Wall -Wextra

# Output binary name (platform-specific)
ifeq ($(OS),Windows_NT)
    TARGET = uf2conv.exe
else
    TARGET = uf2conv
endif

SRC = src/uf2conv.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET) uf2conv uf2conv.exe

# Install to /usr/local/bin (Linux/macOS)
install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/

.PHONY: install
