# Makefile for Arduino sketch compilation using arduino-cli
# Works on Ubuntu and Windows 11
# Generic version - automatically detects sketch name from folder

# Default configuration
DEFAULT_FQBN := STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE

# Allow FQBN to be overridden from command line
FQBN ?= $(DEFAULT_FQBN)

# Detect OS for cross-platform compatibility
ifeq ($(OS),Windows_NT)
    # Windows
    ARDUINO_CLI := arduino-cli.exe
    CURRENT_DIR := $(shell cd)
    SKETCH_NAME := $(notdir $(shell cd))
    ECHO_CMD := echo
    CACHE_DIR := $(LOCALAPPDATA)/Arduino15/cache
    RM_CMD := rmdir /s /q
else
    # Linux/Ubuntu
    ARDUINO_CLI := arduino-cli
    CURRENT_DIR := $(shell pwd)
    SKETCH_NAME := $(notdir $(shell pwd))
    ECHO_CMD := echo
    CACHE_DIR := $(HOME)/.cache/arduino
    RM_CMD := rm -rf
endif

# Default target
.PHONY: all
all: compile

# Compile the sketch
.PHONY: compile
compile:
	@$(ECHO_CMD) "Compiling sketch: $(SKETCH_NAME)"
	@$(ECHO_CMD) "In directory: $(CURRENT_DIR)"
	@$(ECHO_CMD) "Using FQBN: $(FQBN)"
	$(ARDUINO_CLI) compile --fqbn "$(FQBN)" "$(CURRENT_DIR)" --verbose
	@$(ECHO_CMD) "Compilation of $(SKETCH_NAME) completed!"

# Clean build artifacts
.PHONY: clean
clean:
	@$(ECHO_CMD) "Cleaning build artifacts..."
	@$(ECHO_CMD) "Removing cached build files..."
	-$(RM_CMD) "$(CACHE_DIR)/sketches"
	-$(RM_CMD) "$(CACHE_DIR)/cores"
	@$(ECHO_CMD) "Clean completed!"

# Upload to board (requires board to be connected)
.PHONY: upload
upload: compile
	@$(ECHO_CMD) "Uploading $(SKETCH_NAME) to board with FQBN: $(FQBN)"
	$(ARDUINO_CLI) upload --fqbn "$(FQBN)" "$(CURRENT_DIR)" --verbose

# Verify arduino-cli installation
.PHONY: check
check:
	@$(ECHO_CMD) "Checking arduino-cli installation..."
	$(ARDUINO_CLI) version
	@$(ECHO_CMD) "arduino-cli is available!"

# List available boards
.PHONY: list-boards
list-boards:
	@$(ECHO_CMD) "Available boards:"
	$(ARDUINO_CLI) board listall

# List connected boards
.PHONY: list-connected
list-connected:
	@$(ECHO_CMD) "Connected boards:"
	$(ARDUINO_CLI) board list

# Install/update core for STM32
.PHONY: install-core
install-core:
	@$(ECHO_CMD) "Installing/updating STM32 core..."
	$(ARDUINO_CLI) core update-index
	$(ARDUINO_CLI) core install STMicroelectronics:stm32

# Show help
.PHONY: help
help:
	@$(ECHO_CMD) "Available targets:"
	@$(ECHO_CMD) "  compile      - Compile the sketch (default)"
	@$(ECHO_CMD) "  clean        - Clean build artifacts"
	@$(ECHO_CMD) "  upload       - Compile and upload to board"
	@$(ECHO_CMD) "  check        - Verify arduino-cli installation"
	@$(ECHO_CMD) "  list-boards  - List all available boards"
	@$(ECHO_CMD) "  list-connected - List connected boards"
	@$(ECHO_CMD) "  install-core - Install/update STM32 core"
	@$(ECHO_CMD) "  help         - Show this help"
	@$(ECHO_CMD) ""
	@$(ECHO_CMD) "Usage examples:"
	@$(ECHO_CMD) "  make                    # Compile with default FQBN"
	@$(ECHO_CMD) "  make compile            # Same as above"
	@$(ECHO_CMD) "  make FQBN=arduino:avr:uno  # Compile for Arduino Uno"
	@$(ECHO_CMD) "  make upload             # Compile and upload"
	@$(ECHO_CMD) "  make clean              # Clean build artifacts"
	@$(ECHO_CMD) ""
	@$(ECHO_CMD) "Current sketch: $(SKETCH_NAME)"
	@$(ECHO_CMD) "Current FQBN: $(FQBN)"
