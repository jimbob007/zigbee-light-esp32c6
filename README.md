ESP32-C6 Zigbee Dimmable Light - Technical Overview
================================================

Hardware Configuration
--------------------
Board: Seeed XIAO ESP32C6
Power: 2x 18650 Li-ion batteries in parallel
Connections:
- LED: GPIO21 (D10) - PWM controlled
- Button: GPIO1 (D1) - Input with pulldown
- Battery Voltage: ADC Channel 0 (A0) - Voltage divider

Project Structure
---------------
/zigbee_light_v0.55/
├── include/                  # Header files
│   ├── Button.h             # Button handling class
│   ├── config.h             # Centralized configuration
│   ├── globals.h            # Global state declarations
│   └── ZigbeeDimmableLight.h # Zigbee device interface
├── src/                     # Source files
│   ├── main.cpp             # Application entry point
│   ├── globals.cpp          # Global variable implementations
│   ├── functions.cpp        # Core functionality
│   └── ZigbeeDimmableLight.cpp # Zigbee implementation
├── platformio.ini           # PlatformIO configuration
└── partitions.csv           # Custom partition table

Technical Implementation
----------------------
1. Battery Management:
   - Voltage range: 3.0V - 4.2V per cell
   - Median filtering with 8 samples
   - EMA smoothing (alpha = 0.3)
   - Configurable reporting thresholds
   - Sleep current optimization

2. LED Control:
   - 8-bit PWM (1.8 kHz)
   - Gamma correction table
   - Dimming range: 0-255
   - Smooth transitions

3. Network:
   - Zigbee End Device
   - Standard clusters:
     * On/Off
     * Level Control
     * Power Configuration
   - Attribute reporting
   - Network stability features

4. Power States:
   - Active: Full operation
   - Sleep: Timer or button wake
   - Deep sleep: Background monitoring
   - Emergency shutdown on low voltage

Configuration Reference
---------------------
1. Battery Settings:
   - CHECK_INTERVAL_MS: 30000
   - EMA_ALPHA: 0.3
   - LOW_THRESHOLD: 15%
   - REPORT_CHANGE: 5%

2. Sleep Settings:
   - SLEEP_DELAY_MS: 30000
   - WAKE_INTERVAL_S: 30
   - FACTORY_RESET_TIME_MS: 5000

3. PWM Configuration:
   - FREQ: 1800 Hz
   - RESOLUTION: 8-bit
   - GAMMA_CORRECTION: Enabled

Build Requirements
----------------
- Platform: espressif32
- Board: seeed_xiao_esp32c6
- Frameworks: arduino, espidf
- Components:
  * driver
  * esp_adc
  * esp_timer
  * esp_zigbee
  * nvs_flash

Known Limitations
---------------
1. Hardware:
   - Maximum LED current: 20mA
   - Battery measurement accuracy: ±0.1V
   - Button debounce time: 50ms

2. Software:
   - Wake-up time: ~500ms
   - Attribute update delay: ~100ms
   - Memory constraints
   - Stack size limits

Troubleshooting
--------------
1. Battery Issues:
   - Verify voltage divider
   - Check cell balance
   - Monitor discharge curve
   - Validate ADC readings

2. Network Issues:
   - Check signal strength
   - Monitor reconnection attempts
   - validate cluster implementations
