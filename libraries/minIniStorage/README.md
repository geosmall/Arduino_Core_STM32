# minIniStorage - INI Configuration with Storage Abstraction

INI file configuration management integrated with the Storage abstraction layer. Automatically selects LittleFS or SDFS backend based on board configuration.

## Architecture

```
┌─────────────────────────────────────────┐
│            Your Application             │
│   config.put("section", "key", value)   │
└────────────────┬────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────┐
│           minIniStorage                 │
│  • Storage initialization wrapper       │
│  • C++ std::string API                  │
└────────────────┬────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────┐
│              minIni                     │
│  • INI file parsing (CompuPhase)        │
│  • Read/write/delete operations         │
└────────────────┬────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────┐
│           StorageGlue.h                 │
│  • Maps minIni file I/O to Storage API  │
│  • Float formatting via libPrintf       │
└────────────────┬────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────┐
│     Storage → LittleFS or SDFS          │
└─────────────────────────────────────────┘
```

## Quick Start

```cpp
#include <minIniStorage.h>
#include "targets/NUCLEO_F411RE_LITTLEFS.h"

minIniStorage config("settings.ini");

void setup() {
    // Initialize with board configuration
    if (config.begin(BoardConfig::storage)) {
        // Write values
        config.put("network", "ip", "192.168.1.100");
        config.put("network", "port", 8080);
        config.put("sensor", "offset", 2.5f);
        config.put("flags", "enabled", true);

        // Read values (with defaults)
        std::string ip = config.gets("network", "ip", "0.0.0.0");
        int port = config.geti("network", "port", 80);
        float offset = config.getf("sensor", "offset", 0.0f);
        bool enabled = config.getbool("flags", "enabled", false);
    }
}
```

## INI File Format

```ini
[network]
ip_address=192.168.1.100
port=8080
dhcp_enabled=true

[sensor]
temperature_offset=2.500000
sample_rate=1000
enabled=true
```

## Dependencies

- **Storage** - Unified storage abstraction
- **libPrintf** - Float formatting (used internally by StorageGlue.h)

## Credits

Based on [minIni](https://github.com/compuphase/minIni) v1.5 by CompuPhase (Apache 2.0 License).

## Example

See `examples/minIniStorage_Example/` for a complete demonstration including:
- Writing configuration values
- Reading with defaults
- Section/key enumeration
- v1.5 features (hassection, haskey)
