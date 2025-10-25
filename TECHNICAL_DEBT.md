# Technical Debt and Future Improvements

## v1.0.1 Planned Improvements

### Platform.txt - Version-Agnostic Tool References

**Priority**: Medium
**Effort**: Low (4 line changes)
**Risk**: Low (backward compatible)

**Current State** (v1.0.0):
```properties
# Hardcoded version numbers in platform.txt
compiler.path={runtime.tools.xpack-arm-none-eabi-gcc-12.2.1-1.2.path}/bin/
compiler.arm.cmsis.c.flags="-I{runtime.tools.CMSIS-5.9.0.path}/..."
debug.server.openocd.path={runtime.tools.xpack-openocd-0.12.0-1.path}/bin/openocd
debug.server.openocd.scripts_dir={runtime.tools.xpack-openocd-0.12.0-1.path}/openocd/scripts
```

**Desired State** (v1.0.1):
```properties
# Version-agnostic references (automatically resolve to installed version)
compiler.path={runtime.tools.xpack-arm-none-eabi-gcc.path}/bin/
compiler.arm.cmsis.c.flags="-I{runtime.tools.CMSIS.path}/..."
debug.server.openocd.path={runtime.tools.xpack-openocd.path}/bin/openocd
debug.server.openocd.scripts_dir={runtime.tools.xpack-openocd.path}/openocd/scripts
```

**Benefits**:
- Tool version updates only require Board Manager JSON changes (no platform.txt edits)
- Matches STM32duino best practices
- More maintainable and future-proof
- No functional change for users

**Implementation**:
1. Update 4 lines in platform.txt (lines 26, 78, 220, 221)
2. Test compilation with version-agnostic paths
3. Rebuild release archive
4. Update Board Manager package

**Files to Modify**:
- `platform.txt` (4 lines)

**Testing Required**:
- Verify compilation works with version-agnostic paths
- Test on Windows, macOS, Linux
- Confirm debugger (OpenOCD) paths resolve correctly

**Documentation**:
- Add note to release notes about improved tool version flexibility
