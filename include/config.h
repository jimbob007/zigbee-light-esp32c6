#pragma once

#include <driver/gpio.h>
#include <driver/ledc.h>

namespace Config {
    namespace Pins {
        constexpr gpio_num_t LED = GPIO_NUM_21;        // LED output pin
        constexpr gpio_num_t BUTTON = GPIO_NUM_2;      // Button input pin
        constexpr adc_channel_t VOLTAGE_SENSE = ADC_CHANNEL_2;  // Battery voltage sensing
    }

    namespace PWM {
        constexpr ledc_mode_t MODE = LEDC_LOW_SPEED_MODE;
        constexpr ledc_timer_t TIMER = LEDC_TIMER_0;
        constexpr ledc_channel_t CHANNEL = LEDC_CHANNEL_0;
        constexpr uint32_t FREQ = 5000;  // PWM frequency in Hz
    }

    namespace Battery {
        constexpr uint32_t CHECK_INTERVAL_MS = 60000;  // Battery check interval
        constexpr uint32_t NUM_SAMPLES = 16;          // Number of ADC samples to average
        constexpr float EMA_ALPHA = 0.3f;            // EMA filter coefficient
        constexpr uint32_t MIN_MV = 2700;           // Minimum battery voltage (2.7V)
        constexpr uint32_t MAX_MV = 4200;           // Maximum battery voltage (4.2V)
        constexpr uint8_t LOW_THRESHOLD = 20;        // Low battery warning threshold (%)
        constexpr uint8_t REPORT_CHANGE = 5;         // Report if changed by this percentage
    }

    namespace Sleep {
        constexpr uint32_t DELAY_MS = 300000;        // Sleep after 5 minutes of inactivity
        constexpr uint32_t WAKE_INTERVAL_S = 3600;   // Wake every hour to check battery
        constexpr uint32_t FACTORY_RESET_TIME_MS = 10000;  // Factory reset after 10s button press
    }

    namespace Zigbee {
        constexpr uint8_t ENDPOINT = 10;             // Zigbee endpoint number
    }
}