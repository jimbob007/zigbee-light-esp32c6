#pragma once

#include <Arduino.h>
#include <esp_zigbee_core.h>

namespace Functions {
    bool initHardware();
    void setLED(bool state);
    void setLEDLevel(uint8_t level);
    void checkVoltage();
    void enterSleep();
    bool confirmFactoryReset();
    void toggleLight();
    void checkAndEnterSleep();
    void zbAttributeChangedCallback(esp_zb_zcl_set_attr_value_message_t *message);
    void zbStatusCallback(esp_zb_zcl_status_message_t *message);
}