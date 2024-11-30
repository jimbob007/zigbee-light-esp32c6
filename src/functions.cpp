#include "globals.h"
#include "config.h"
#include "esp_sleep.h"

namespace Functions {

// Previous functions remain the same...

void enterSleep() {
    Serial.println("Preparing for sleep...");
    
    const uint64_t pin_mask = (1ULL << Config::Pins::BUTTON);
    esp_sleep_enable_ext1_wakeup(pin_mask, ESP_EXT1_WAKEUP_ANY_HIGH);
    esp_sleep_enable_timer_wakeup(Config::Sleep::WAKE_INTERVAL_S * 1000000ULL);
    
    ledc_stop(Config::PWM::MODE, Config::PWM::CHANNEL, 0);
    
    esp_zb_sleep_now();
    
    Serial.println("Entering deep sleep");
    Serial.flush();
    
    esp_deep_sleep_start();
}

bool confirmFactoryReset() {
    uint8_t flashes = 5;
    while (flashes--) {
        digitalWrite(Config::Pins::LED, HIGH);
        delay(100);
        digitalWrite(Config::Pins::LED, LOW);
        delay(100);
    }
    return true;
}

void toggleLight() {
    bool current_state = false;
    esp_zb_zcl_attr_t *attr = esp_zb_zcl_get_attribute(
        Config::Zigbee::ENDPOINT, 
        ESP_ZB_ZCL_CLUSTER_ID_ON_OFF, 
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE, 
        ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID);
    if (attr) {
        current_state = *(bool*)attr->data_p;
        setLED(!current_state);
    }
}

void checkAndEnterSleep() {
    bool is_on = false;
    esp_zb_zcl_attr_t *attr = esp_zb_zcl_get_attribute(
        Config::Zigbee::ENDPOINT, 
        ESP_ZB_ZCL_CLUSTER_ID_ON_OFF, 
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE, 
        ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID);
    if (attr) {
        is_on = *(bool*)attr->data_p;
    }
    
    if (!is_on && esp_zb_is_network_up()) {
        enterSleep();
    }
}

void zbAttributeChangedCallback(esp_zb_zcl_set_attr_value_message_t *message) {
    if (Globals::zbLight) {
        Globals::zbLight->handleAttributeUpdate(message);
    }
}

void zbStatusCallback(esp_zb_zcl_status_message_t *message) {
    if (!message) return;
    
    switch (message->status) {
        case ESP_ZB_ZCL_STATUS_SUCCESS:
            Serial.println("Zigbee operation successful");
            break;
        case ESP_ZB_ZCL_STATUS_FAIL:
            Serial.println("Zigbee operation failed");
            break;
        default:
            Serial.printf("Zigbee status: %d\n", message->status);
            break;
    }
}

} // namespace Functions