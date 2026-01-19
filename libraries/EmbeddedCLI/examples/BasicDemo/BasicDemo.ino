/**
 * EmbeddedCLI Basic Demo
 *
 * Demonstrates the C++ wrapper API with:
 * - Command history (UP/DOWN arrows)
 * - Line editing (LEFT/RIGHT, HOME, END, BACKSPACE, DELETE)
 * - History search (Ctrl-R)
 * - Built-in command registration
 *
 * Terminal: 115200 baud, use VT100-compatible terminal (PuTTY, minicom, screen)
 */

#include <EmbeddedCLI.h>

// Create CLI instance attached to Serial
EmbeddedCLI cli(Serial);

// Command handlers - declared before use
void cmd_help(int argc, char** argv);
void cmd_led(int argc, char** argv);
void cmd_info(int argc, char** argv);
void cmd_echo(int argc, char** argv);

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);

    pinMode(LED_BUILTIN, OUTPUT);

    Serial.println("\nEmbeddedCLI Demo");
    Serial.println("Type 'help' for available commands\n");

    // Initialize CLI with prompt
    cli.begin("> ");

    // Register commands
    cli.addCommand("help",  "Show available commands",   cmd_help);
    cli.addCommand("led",   "Control LED: led <on|off>", cmd_led);
    cli.addCommand("info",  "Show board information",    cmd_info);
    cli.addCommand("echo",  "Echo arguments back",       cmd_echo);
}

void loop() {
    // Process CLI input - handles everything automatically
    cli.process();
}

// Command implementations

void cmd_help(int argc, char** argv) {
    (void)argc; (void)argv;
    cli.printHelp();
}

void cmd_led(int argc, char** argv) {
    if (argc != 2) {
        cli.println("Usage: led <on|off>");
        return;
    }
    if (strcmp(argv[1], "on") == 0) {
        digitalWrite(LED_BUILTIN, HIGH);
        cli.println("LED ON");
    } else if (strcmp(argv[1], "off") == 0) {
        digitalWrite(LED_BUILTIN, LOW);
        cli.println("LED OFF");
    } else {
        cli.println("Usage: led <on|off>");
    }
}

void cmd_info(int argc, char** argv) {
    (void)argc; (void)argv;
    Stream& out = cli.getStream();
    out.print("Board: ");
#ifdef BOARD_NAME
    out.println(BOARD_NAME);
#else
    out.println("Unknown");
#endif
    out.print("Clock: ");
    out.print(SystemCoreClock / 1000000);
    out.println(" MHz");
    out.print("Uptime: ");
    out.print(millis() / 1000);
    out.println(" seconds");
}

void cmd_echo(int argc, char** argv) {
    Stream& out = cli.getStream();
    for (int i = 1; i < argc; i++) {
        if (i > 1) out.print(" ");
        out.print(argv[i]);
    }
    out.println();
}
