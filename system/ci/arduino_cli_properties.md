# Arduino-CLI Property Cheatsheet & Shell Helpers

A copy-pasteable guide to discover and use all the `key=value` properties exposed by `arduino-cli` for scripting and CI.

> Tested with `arduino-cli` ≥ 1.0 on Linux. Commands are POSIX `sh` unless noted.

---

## Project default

Use this default FQBN across all examples and helpers:

```sh
# Set once in your shell profile or CI environment:
export FQBN="STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE"
```

All helper functions below accept an optional FQBN argument; if omitted, they fall back to `$FQBN`.

---

## Quick Start: dump every property for a build

From your sketch folder:

```sh
arduino-cli compile   --fqbn "$FQBN"   --show-properties=expanded
```

- `--show-properties=expanded` prints one `key=value` per line with all `{vars}` fully resolved to absolute paths/flags.  
- Use `--show-properties=unexpanded` to see raw variables as defined in `platform.txt`.

**Nice filter (only the “interesting” build keys):**
```sh
arduino-cli compile --fqbn "$FQBN" --show-properties=expanded |
  grep -E '^(build|compiler|upload|tools|recipe|runtime)\.' | sort
```

---

## Board options (FQBN menus) & board-level properties

Discover valid board option keys/values and board properties:

```sh
# Human-readable:
arduino-cli board details -b "$FQBN" --full

# Machine-readable JSON:
arduino-cli board details -b "$FQBN" --format json

# As key=value (expanded):
arduino-cli board details -b "$FQBN" --show-properties=expanded
```

Use this to find the exact `menu` keys you can append to the FQBN (e.g., `:usb=CDC_GENERIC:upload_method=STLink` on some cores).

---

## Where properties come from (the canonical source)

Properties and recipes live in your core’s `platform.txt` (and optional `platform.local.txt`):

```sh
DATA_DIR=$(arduino-cli config get directories.data)
# Example core path (auto-detect version with a wildcard):
grep -nE '^[A-Za-z0-9_.-]+\s*='   "$DATA_DIR/packages/STMicroelectronics/hardware/stm32/"*/platform.txt
```

Other useful files:
- `boards.txt` (board IDs, menus)
- `programmers.txt` (uploaders)

---

## CLI configuration keys (for scripting the CLI itself)

```sh
# Show all user-set values (not all defaults):
arduino-cli config dump

# Query a single key (shows default if unset):
arduino-cli config get directories.data
arduino-cli config get board_manager.additional_urls
```

---

## Shell helpers (drop-in functions)

### 1) Get the core/platform folder (absolute path)

```sh
# Returns path that contains platform.txt, boards.txt, etc.
# Usage: CORE_DIR=$(get_core_dir)           # uses $FQBN
#    or: CORE_DIR=$(get_core_dir "$FQBN")  # explicit
get_core_dir() {
  fqbn=${1:-$FQBN}
  arduino-cli board details -b "$fqbn" --show-properties=expanded 2>/dev/null |
    awk -F= '/^runtime\.platform\.path=/{sub(/^runtime\.platform\.path=/,""); print; exit}'
}
```

Alternative (from a sketch dir, using compile context):
```sh
get_core_dir_from_compile() {
  fqbn=${1:-$FQBN}
  arduino-cli compile --fqbn "$fqbn" --show-properties=expanded 2>/dev/null |
    awk -F= '/^runtime\.platform\.path=/{sub(/^runtime\.platform\.path=/,""); print; exit}'
}
```

Pure path math (good for CI; requires `jq`):
```sh
# Usage: CORE_DIR=$(get_core_dir_fast)           # uses $FQBN
#    or: CORE_DIR=$(get_core_dir_fast "$FQBN")  # explicit
get_core_dir_fast() {
  fqbn=${1:-$FQBN}
  data_dir=$(arduino-cli config get directories.data)
  pkg=$(printf '%s' "$fqbn" | cut -d: -f1)
  arch=$(printf '%s' "$fqbn" | cut -d: -f2)
  ver=$(arduino-cli core list --format json | jq -r --arg id "$pkg:$arch"         '.installed[] | select(.id==$id) | .version')
  printf '%s/packages/%s/hardware/%s/%s\n' "$data_dir" "$pkg" "$arch" "$ver"
}
```

### 2) Dump a clean `props.mk` file for Make/bash use

```sh
# Writes every expanded property for your FQBN into props.mk
# Usage: dump_props_mk > props.mk          # uses $FQBN
#    or: dump_props_mk "$FQBN" > props.mk
dump_props_mk() {
  fqbn=${1:-$FQBN}
  arduino-cli compile --fqbn "$fqbn" --show-properties=expanded 2>/dev/null |
    # Keep raw keys/values; Make can read VAR=VALUE lines even with dots
    sed 's/\r$//' 
}
```

### 3) List valid menu options (FQBN “key=value” choices)

```sh
# Prints all board menu keys and allowed values for your FQBN
# Usage: list_board_menus            # uses $FQBN
#    or: list_board_menus "$FQBN"
list_board_menus() {
  fqbn=${1:-$FQBN}
  arduino-cli board details -b "$fqbn" --full |
    awk '
      /^Options:/{inopt=1; next}
      inopt && NF==0 {inopt=0}
      inopt {print}
    '
}
```

---

## Frequently used property keys (grab with grep)

- **Core & environment**
  - `runtime.platform.path`
  - `runtime.os`, `runtime.ide.version`
  - `build.arch`, `build.core`, `build.system.path`, `build.path`, `build.project_name`

- **Compiler toolchain**
  - `compiler.path`, `compiler.c.cmd`, `compiler.cpp.cmd`, `compiler.ar.cmd`, `compiler.c.elf.cmd`
  - `compiler.c.flags`, `compiler.cpp.flags`, `compiler.S.flags`, `compiler.ar.flags`, `compiler.c.elf.flags`, `compiler.libraries.ldflags`

- **Include & library paths**
  - `includes`, `build.vm.includes`
  - `runtime.tools.*` (resolved locations of tools)

- **Uploader / tools**
  - `tools.*.path`, `tools.*.cmd`, `tools.*.pattern`
  - `upload.tool`, `upload.protocol`, `upload.port`, `upload.maximum_size`

- **Recipes (actual command lines)**
  - `recipe.c.o.pattern`, `recipe.cpp.o.pattern`, `recipe.ar.pattern`, `recipe.c.combine.pattern`, `recipe.objcopy.eep.pattern`, etc.

**Example filter:**
```sh
arduino-cli compile --fqbn "$FQBN" --show-properties=expanded |
  grep -E '^(runtime\.platform\.path|compiler\.path|recipe\.|upload\.|tools\.)'
```

---

## Examples

**Print the core directory path:**
```sh
CORE_DIR=$(get_core_dir)
printf 'Core dir: %s\n' "$CORE_DIR"
```

**Generate a properties file for your build job:**
```sh
dump_props_mk > props.mk
. ./props.mk   # bash: imports as shell vars if keys are vanilla; otherwise grep what you need
```

**Find the exact upload command that will run:**
```sh
arduino-cli compile --fqbn "$FQBN" --show-properties=expanded |
  grep '^recipe.upload.pattern=' | cut -d= -f2-
```

---

## Tips & gotchas

- **Expanded vs unexpanded**: Prefer `expanded` for scripts (absolute paths). Use `unexpanded` to understand how recipes are constructed in `platform.txt`.
- **Board options matter**: Properties can change with FQBN menu selections (`:speed=…`, `:usb=…`, etc.). Always pass the *full* FQBN you intend to use in CI.
- **`config dump`**: Shows only user-set values; use `config get <key>` to see defaults (e.g., `directories.data`).
- **Platforms override**: `platform.local.txt` (if present) augments/overrides `platform.txt`. Your dumped properties already reflect the final resolved values.
- **Windows paths**: If you port these scripts, quote variables aggressively (`"$VAR"`) to survive spaces in paths.

---

## Appendix: minimal dependency version check

```sh
arduino-cli version
# If you rely on jq-based helpers:
jq --version
```
