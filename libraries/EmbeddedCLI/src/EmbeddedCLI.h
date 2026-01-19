/**
 * EmbeddedCLI Arduino Library (Header-only C++ wrapper)
 *
 * Based on AndreRenaud/EmbeddedCLI (0BSD License).
 *
 * Usage:
 *   #include <EmbeddedCLI.h>
 *
 *   EmbeddedCLI cli(Serial);
 *
 *   void setup() {
 *       Serial.begin(115200);
 *       cli.begin("> ");
 *       cli.addCommand("help", "Show commands", cmd_help);
 *   }
 *
 *   void loop() {
 *       cli.process();
 *   }
 */
#ifndef EMBEDDEDCLI_ARDUINO_H
#define EMBEDDEDCLI_ARDUINO_H

#include <Arduino.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "embedded_cli.h"

#ifdef __cplusplus
}
#endif

// Maximum number of registered commands
#ifndef EMBEDDEDCLI_MAX_COMMANDS
#define EMBEDDEDCLI_MAX_COMMANDS 16
#endif

/**
 * Command callback function type
 * @param argc Number of arguments (including command name)
 * @param argv Array of argument strings
 */
typedef void (*CLICommandHandler)(int argc, char** argv);

/**
 * EmbeddedCLI - Arduino C++ wrapper class
 *
 * Provides a clean object-oriented interface for command-line processing.
 */
class EmbeddedCLI {
public:
    /**
     * Constructor
     * @param stream Serial port or other Stream for I/O
     */
    EmbeddedCLI(Stream& stream)
        : _stream(stream)
        , _commandCount(0)
        , _initialized(false)
    {
        memset(_commands, 0, sizeof(_commands));
    }

    /**
     * Initialize the CLI
     * @param prompt Command prompt string (default: "> ")
     */
    void begin(const char* prompt = "> ") {
        embedded_cli_init(&_cli, prompt, putcharCallback, this);
        embedded_cli_prompt(&_cli);
        _initialized = true;
    }

    /**
     * Register a command
     * @param name Command name (what user types)
     * @param help Help text for the command
     * @param handler Function to call when command is entered
     * @return true if command was added, false if table is full
     */
    bool addCommand(const char* name, const char* help, CLICommandHandler handler) {
        if (_commandCount >= EMBEDDEDCLI_MAX_COMMANDS) {
            return false;
        }
        _commands[_commandCount].name = name;
        _commands[_commandCount].help = help;
        _commands[_commandCount].handler = handler;
        _commandCount++;
        return true;
    }

    /**
     * Process incoming characters and execute commands
     * Call this in loop()
     */
    void process() {
        if (!_initialized) return;

        while (_stream.available()) {
            char ch = _stream.read();
            if (embedded_cli_insert_char(&_cli, ch)) {
                int argc;
                char** argv;
                argc = embedded_cli_argc(&_cli, &argv);
                if (argc > 0) {
                    dispatchCommand(argc, argv);
                }
                embedded_cli_prompt(&_cli);
            }
        }
    }

    /**
     * Print help for all registered commands
     */
    void printHelp() {
        _stream.println("Available commands:");
        for (uint8_t i = 0; i < _commandCount; i++) {
            _stream.print("  ");
            _stream.print(_commands[i].name);
            _stream.print("\t- ");
            _stream.println(_commands[i].help);
        }
    }

    /**
     * Print a message to the CLI output
     */
    void print(const char* str) { _stream.print(str); }
    void println(const char* str) { _stream.println(str); }
    void println() { _stream.println(); }

    /**
     * Get the underlying Stream
     */
    Stream& getStream() { return _stream; }

    /**
     * Get command history entry
     * @param pos 0 = most recent, 1 = previous, etc.
     * @return Command string or nullptr if not available
     */
    const char* getHistory(int pos) {
        return embedded_cli_get_history(&_cli, pos);
    }

    /**
     * Get the raw CLI structure (for advanced use)
     */
    struct embedded_cli* getRawCLI() { return &_cli; }

private:
    struct Command {
        const char* name;
        const char* help;
        CLICommandHandler handler;
    };

    static void putcharCallback(void* data, char ch, bool is_last) {
        EmbeddedCLI* self = static_cast<EmbeddedCLI*>(data);
        self->_stream.write(ch);
        if (is_last) {
            self->_stream.flush();
        }
    }

    void dispatchCommand(int argc, char** argv) {
        for (uint8_t i = 0; i < _commandCount; i++) {
            if (strcmp(argv[0], _commands[i].name) == 0) {
                _commands[i].handler(argc, argv);
                return;
            }
        }
        _stream.print("Unknown command: ");
        _stream.println(argv[0]);
        _stream.println("Type 'help' for available commands.");
    }

    Stream& _stream;
    struct embedded_cli _cli;
    Command _commands[EMBEDDEDCLI_MAX_COMMANDS];
    uint8_t _commandCount;
    bool _initialized;
};

#endif // EMBEDDEDCLI_ARDUINO_H
