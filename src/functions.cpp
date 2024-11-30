#include "globals.h"
#include "config.h"
#include "esp_sleep.h"

namespace Functions {

bool initHardware() {
    // Configure GPIO pins
    pinMode(Config::Pins::LED, OUTPUT);
    pinMode(Config::Pins::BUTTON, INPUT);
    
    // Initialize ADC for battery monitoring
    adc_oneshot_unit_init_cfg_t adc_config = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE
    };
    if (adc_oneshot_new_unit(&adc_config, &Globals::adc_handle) != ESP_OK) {
        Serial.println("Failed to initialize ADC!");
        return false;
    }

    // Configure ADC channel
    adc_oneshot_chan_cfg_t chan_config = {
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_DEFAULT
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(Globals::adc_handle, 
        Config::Pins::VOLTAGE_SENSE, &chan_config));

    // Initialize ADC calibration
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_DEFAULT
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_config, &Globals::adc_cali_handle));

    // Initialize LEDC for PWM
    ledc_timer_config_t pwm_timer = {
        .speed_mode = Config::PWM::MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = Config::PWM::TIMER,
        .freq_hz = Config::PWM::FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    if (ledc_timer_config(&pwm_timer) != ESP_OK) {
        Serial.println("Failed to configure LEDC timer!");
        return false;
    }

    ledc_channel_config_t pwm_channel = {
        .gpio_num = Config::Pins::LED,
        .speed_mode = Config::PWM::MODE,
        .channel = Config::PWM::CHANNEL,
        .timer_sel = Config::PWM::TIMER,
        .duty = 0,
        .hpoint = 0
    };
    if (ledc_channel_config(&pwm_channel) != ESP_OK) {
        Serial.println("Failed to configure LEDC channel!");
        return false;
    }

    return true;
}

void setLED(bool state) {
    Serial.printf("Setting LED %s\n", state ? "ON" : "OFF");
    
    uint8_t level = 0;
    esp_zb_zcl_attr_t *attr = esp_zb_zcl_get_attribute(
        Config::Zigbee::ENDPOINT, 
        ESP_ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL, 
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE, 
        ESP_ZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_LEVEL_ID);
        
    if (attr) {
        level = *(uint8_t*)attr->data_p;
    }
    
    ledc_set_duty(Config::PWM::MODE, Config::PWM::CHANNEL, 
        state ? Globals::GAMMA_TABLE[level] : 0);
    ledc_update_duty(Config::PWM::MODE, Config::PWM::CHANNEL);
    
    Globals::last_activity = millis();
}

void setLEDLevel(uint8_t level) {
    Serial.printf("Setting LED level to %d\n", level);
    
    bool is_on = false;
    esp_zb_zcl_attr_t *attr = esp_zb_zcl_get_attribute(
        Config::Zigbee::ENDPOINT, 
        ESP_ZB_ZCL_CLUSTER_ID_ON_OFF, 
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE, 
        ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID);
        
    if (attr) {
        is_on = *(bool*)attr->data_p;
    }
    
    if (is_on) {
        ledc_set_duty(Config::PWM::MODE, Config::PWM::CHANNEL, 
            Globals::GAMMA_TABLE[level]);
        ledc_update_duty(Config::PWM::MODE, Config::PWM::CHANNEL);
    }
    
    Globals::last_activity = millis();
}

void checkVoltage() {
    if (!Globals::adc_handle || !Globals::adc_cali_handle) {
        Serial.println("ADC not initialized!");
        return;
    }

    int32_t samples[Config::Battery::NUM_SAMPLES];
    uint32_t valid_samples = 0;
    
    // Collect samples with validation
    for (int i = 0; i < Config::Battery::NUM_SAMPLES; i++) {
        int32_t raw_value;
        int voltage;
        
        if (adc_oneshot_read(Globals::adc_handle, Config::Pins::VOLTAGE_SENSE, &raw_value) == ESP_OK &&
            adc_cali_raw_to_voltage(Globals::adc_cali_handle, raw_value, &voltage) == ESP_OK) {
            if (voltage > 100 && voltage < 3300) {
                samples[valid_samples++] = voltage;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(2));
    }
    
    if (valid_samples < 4) {
        Serial.printf("Warning: Only %d/%d valid samples\n", valid_samples, Config::Battery::NUM_SAMPLES);
        return;
    }
    
    // Use median filtering
    for (uint32_t i = 0; i < valid_samples-1; i++) {
        for (uint32_t j = i+1; j < valid_samples; j++) {
            if (samples[i] > samples[j]) {
                int32_t temp = samples[i];
                samples[i] = samples[j];
                samples[j] = temp;
            }
        }
    }
    
    // Calculate average using middle samples
    uint32_t voltage_sum = 0;
    uint32_t used_samples = 0;
    for (uint32_t i = valid_samples/4; i < (valid_samples*3)/4; i++) {
        voltage_sum += samples[i];
        used_samples++;
    }
    
    float voltage = (voltage_sum / used_samples) * 2;  // Multiply by 2 for voltage divider
    
    // Apply EMA filter
    if (Globals::voltage_ema == 0) {
        Globals::voltage_ema = voltage;
    } else {
        Globals::voltage_ema = (Config::Battery::EMA_ALPHA * voltage) + 
                             ((1-Config::Battery::EMA_ALPHA) * Globals::voltage_ema);
    }
    
    uint32_t final_voltage = (uint32_t)Globals::voltage_ema;
    Serial.printf("Battery voltage: %.3fV (filtered)\n", final_voltage / 1000.0f);
    
    if (Globals::zbLight) {
        Globals::zbLight->updateBatteryVoltage(final_voltage);
    }
}
