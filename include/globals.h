#pragma once

#include <Arduino.h>
#include <driver/adc.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>
#include "ZigbeeDimmableLight.h"
#include "config.h"
#include "functions.h"

namespace Globals {
    // ADC Handles
    extern adc_oneshot_unit_handle_t adc_handle;
    extern adc_cali_handle_t adc_cali_handle;

    // Battery Monitoring
    extern uint32_t last_voltage_check;
    extern float voltage_ema;

    // Activity Tracking
    extern uint32_t last_activity;

    // Zigbee Instance
    extern ZigbeeDimmableLight* zbLight;

    // Gamma correction lookup table
    extern const uint8_t GAMMA_TABLE[256];
}