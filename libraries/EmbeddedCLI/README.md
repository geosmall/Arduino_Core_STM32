# EmbeddedCLI

Lightweight command-line interface library for embedded systems with no dynamic memory allocation.

Based on [AndreRenaud/EmbeddedCLI](https://github.com/AndreRenaud/EmbeddedCLI) (0BSD License).

## Features

- Command history with UP/DOWN arrow navigation
- History search with Ctrl-R
- Line editing: LEFT/RIGHT arrows, HOME, END, BACKSPACE, DELETE
- Ctrl-U (clear line before cursor), Ctrl-K (clear line after cursor)
- Argument parsing with quoted string support
- No dynamic memory - fixed compile-time sizing
- C++ wrapper class for clean Arduino integration
- Works with any Stream (Serial, SoftwareSerial, etc.)

## C++ API (Recommended)

```cpp
#include <EmbeddedCLI.h>

EmbeddedCLI cli(Serial);

void cmd_help(int argc, char** argv) {
    cli.printHelp();
}

void cmd_led(int argc, char** argv) {
    if (argc == 2 && strcmp(argv[1], "on") == 0) {
        digitalWrite(LED_BUILTIN, HIGH);
        cli.println("LED ON");
    }
}

void setup() {
    Serial.begin(115200);
    cli.begin("> ");
    cli.addCommand("help", "Show commands", cmd_help);
    cli.addCommand("led",  "LED control",   cmd_led);
}

void loop() {
    cli.process();  // Handles input and command dispatch
}
```

## C++ API Reference

```cpp
class EmbeddedCLI {
    EmbeddedCLI(Stream& stream);           // Constructor - attach to any Stream
    void begin(const char* prompt = "> "); // Initialize CLI
    bool addCommand(name, help, handler);  // Register command
    void process();                        // Call in loop() - handles everything
    void printHelp();                      // Print registered commands
    void print(const char* str);           // Output helpers
    void println(const char* str);
    Stream& getStream();                   // Access underlying Stream
    const char* getHistory(int pos);       // Get history (0 = most recent)
};
```

## Raw C API (Advanced)

For lower-level control or custom command dispatch:

```cpp
#include <EmbeddedCLI.h>

static struct embedded_cli cli;

void cli_putchar(void* data, char ch, bool is_last) {
    Serial.write(ch);
    if (is_last) Serial.flush();
}

void setup() {
    Serial.begin(115200);
    embedded_cli_init(&cli, "> ", cli_putchar, nullptr);
    embedded_cli_prompt(&cli);
}

void loop() {
    while (Serial.available()) {
        if (embedded_cli_insert_char(&cli, Serial.read())) {
            int argc;
            char** argv;
            argc = embedded_cli_argc(&cli, &argv);
            // Process command using argv[0], argv[1], etc.
            embedded_cli_prompt(&cli);
        }
    }
}
```

## Memory Usage

Uses upstream defaults (~1.2 KB RAM with history, ~200 bytes without).

## Updating from Upstream

To update from [AndreRenaud/EmbeddedCLI](https://github.com/AndreRenaud/EmbeddedCLI):

1. Download latest `embedded_cli.h` and `embedded_cli.c`
2. Copy to `src/` folder (replacing existing files)

No modifications needed - upstream files work as-is.

## License

0BSD (public domain equivalent) - see upstream repository.
