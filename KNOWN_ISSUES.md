# Known Issues and Planned Improvements

## Current Issues

1. Sleep Mode Configuration
   - Current implementation uses ext1 wakeup which is not supported on ESP32-C6
   - Need to switch to esp_sleep_enable_ext0_wakeup()

2. ADC Configuration
   - ADC unit selection needs verification for ESP32-C6 compatibility
   - Some pins might only be available on ADC_UNIT_2

3. Battery Voltage Calculation
   - Potential precision loss in voltage calculations
   - Integer division without rounding

4. Error Handling
   - Several ESP_ERROR_CHECK calls could cause crashes
   - Need more graceful error handling

5. Memory Management
   - Incomplete cleanup in ZigbeeDimmableLight destructor
   - _cluster_list not being freed

6. Race Conditions
   - Potential race condition in factory reset handling
   - Need atomic operations or mutex

7. Voltage Range Validation
   - Hard-coded voltage ranges
   - Should use configuration constants

8. PWM Resolution
   - Currently using 8-bit resolution
   - Could be improved for smoother dimming

9. Sleep Mode Wake-up Time
   - Magic numbers in time calculations
   - Need defined constants

10. Battery Level Updates
    - Missing hysteresis in battery level updates
    - Could cause frequent updates near thresholds

## Planned Improvements

1. Implement proper sleep mode configuration for ESP32-C6
2. Add proper ADC configuration validation
3. Improve battery voltage calculation precision
4. Implement proper error handling throughout
5. Fix memory management issues
6. Add atomic operations for thread safety
7. Use configuration constants for voltage ranges
8. Implement higher PWM resolution
9. Add proper time constants
10. Implement battery level hysteresis

## Contributing

If you'd like to contribute to fixing any of these issues, please:

1. Create a new branch for your changes
2. Make your changes and test thoroughly
3. Submit a pull request with a clear description of your changes
4. Reference any issues that your changes address

Please check the existing pull requests before starting work to avoid duplicate efforts.