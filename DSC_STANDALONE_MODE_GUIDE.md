# DSC Standalone Mode - Testing Without Connected Panel

## Overview

The DSC Keybus integration now supports a **standalone mode** that allows you to test the ESPHome configuration and hardware setup without requiring a physical DSC alarm panel connection. This is particularly useful for:

- Initial setup and configuration testing
- Hardware development and debugging
- ESP32 module testing before installation
- ESPHome integration validation

## Enabling Standalone Mode

Add `standalone_mode: true` to your DSC keybus configuration:

```yaml
dsc_keybus:
  id: dsc_interface
  access_code: "1234"
  series_type: "Classic"  # or "PowerSeries"
  standalone_mode: true   # Enable standalone mode
  debug: 1               # Enable debug logging to see standalone messages
  # ... other configuration options
```

## What Happens in Standalone Mode

When standalone mode is enabled:

1. **Hardware initialization is simulated** - No actual timers or interrupts are configured
2. **Success is reported immediately** - No waiting for DSC panel communication
3. **All ESPHome features work normally** - WiFi, API, sensors, etc.
4. **Clear logging indicates test mode** - Messages show "standalone mode" in logs
5. **No panel data is processed** - Zone and partition states remain inactive

## Expected Log Output

In standalone mode, you should see logs like:

```
[INFO][dsc_keybus:XXX]: Standalone mode enabled - simulating successful hardware initialization
[INFO][dsc_keybus:XXX]: Note: No actual DSC panel connection required in standalone mode
[INFO][dsc_keybus:XXX]: This mode is for testing ESPHome integration without hardware
[INFO][dsc_keybus:XXX]: DSC Keybus hardware initialization complete (standalone mode)
[INFO][dsc_wrapper:XXX]: DSC Wrapper: Standalone mode enabled - simulating successful hardware initialization
```

## Example Configuration

See `DscAlarm_Standalone_Test.yaml` for a complete example configuration that includes:

- Standalone mode enabled
- Reduced sensor set for testing
- System health monitoring
- Memory diagnostics
- Clear labeling of test entities

## Transitioning from Standalone to Production

When ready to connect to a real DSC panel:

1. **Change `standalone_mode: false`** (or remove the line as false is default)
2. **Verify wiring connections** match your configuration
3. **Update entity names** to remove "(Test)" suffixes if desired
4. **Flash the updated configuration**
5. **Monitor logs** for successful hardware initialization

## Troubleshooting

### Standalone Mode Not Working

If you see hardware initialization errors even with standalone mode enabled:

- Verify the configuration syntax is correct
- Check that you're using the latest component version
- Ensure `standalone_mode: true` is properly indented under `dsc_keybus:`

### False Hardware Errors

If you see errors like:
```
DSC hardware initialization status unclear (attempt X/5)
DSC hardware initialization failed permanently after multiple attempts
```

This means standalone mode is not enabled. Add `standalone_mode: true` to your configuration.

## Configuration Files

- **Production**: `DscAlarm.yaml` - Default configuration for connected panels
- **Testing**: `DscAlarm_Standalone_Test.yaml` - Standalone mode example
- **Classic**: `DscAlarm_Classic.yaml` - Classic series with optional standalone mode

## Hardware Requirements in Standalone Mode

In standalone mode, you only need:

- ESP32 development board
- WiFi connection
- Power supply
- No DSC panel connection required
- No resistors or wiring required

## Next Steps

After successful standalone testing:

1. Review the [Complete Setup Guide](ESPHOME_COMPLETE_SETUP_GUIDE.md) for wiring instructions
2. Follow the hardware wiring diagrams for your DSC panel type
3. Use the production configuration files for actual panel integration
4. Test with your physical DSC alarm system

## Support

If you encounter issues with standalone mode, please include:

- Your complete ESPHome configuration
- Full log output from device boot
- ESP32 board type and ESPHome version
- Whether you're testing with or without hardware connections