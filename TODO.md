# TODO - Future Improvements

## ci_log.h Improvements (Separate PR)

### Improve CI_LOGF Float Support for RTT
Current CI_LOGF doesn't support float formatting in RTT mode (SEGGER_RTT_printf limitation). Consider:

1. **Option A**: Document limitation clearly and recommend CI_LOG_FLOAT
2. **Option B**: Add compile-time warning when float format specifiers detected
3. **Option C**: Wrapper that automatically detects %f and routes to CI_LOG_FLOAT

**Current Workaround**:
```cpp
// Instead of: CI_LOGF("Value: %.2f\n", value);
CI_LOG_FLOAT("Value: ", value, 2);
CI_LOG("\n");
```

### Add ci_log.h Reference to CLAUDE.md
After ci_log.h improvements are complete, add API reference section to CLAUDE.md with:
- Available macros and their usage
- Setup patterns for Serial vs RTT modes
- Format specifier limitations and workarounds
- Integration with HIL framework
