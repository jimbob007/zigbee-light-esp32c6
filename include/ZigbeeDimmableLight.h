#pragma once

#include <Arduino.h>
#include <esp_zigbee_core.h>
#include "config.h"

class ZigbeeDimmableLight {
public:
    ZigbeeDimmableLight(uint8_t endpoint);
    ~ZigbeeDimmableLight();

    bool begin();
    void handleAttributeUpdate(esp_zb_zcl_set_attr_value_message_t *message);
    void onLightChange(void (*callback)(bool));
    void onLevelChange(void (*callback)(uint8_t));
    void updateBatteryVoltage(uint32_t millivolts);
    
    esp_zb_ep_list_t* getEndpointList() { return _ep_list; }

private:
    bool initBasicCluster();
    bool initOnOffCluster();
    bool initPowerConfigCluster();
    bool initLevelControlCluster();

    uint8_t _endpoint;
    esp_zb_ep_list_t* _ep_list;
    esp_zb_cluster_list_t* _cluster_list;
    
    void (*_on_light_change)(bool);
    void (*_on_level_change)(uint8_t);
    
    bool _current_state;
    uint8_t _current_level;
    uint32_t _battery_voltage;
    uint8_t _last_reported_percentage;
    bool _low_battery_reported;
};