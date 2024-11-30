#include "ZigbeeDimmableLight.h"

ZigbeeDimmableLight::ZigbeeDimmableLight(uint8_t endpoint)
    : _endpoint(endpoint),
      _ep_list(nullptr),
      _cluster_list(nullptr),
      _on_light_change(nullptr),
      _on_level_change(nullptr),
      _current_state(false),
      _current_level(0),
      _battery_voltage(0),
      _last_reported_percentage(0),
      _low_battery_reported(false) {
}

ZigbeeDimmableLight::~ZigbeeDimmableLight() {
    if (_ep_list) {
        esp_zb_ep_list_free(_ep_list);
    }
}

bool ZigbeeDimmableLight::begin() {
    // Create cluster list
    _cluster_list = esp_zb_zcl_cluster_list_create();
    if (!_cluster_list) {
        log_e("Failed to create cluster list");
        return false;
    }

    // Initialize clusters
    if (!initBasicCluster() || !initPowerConfigCluster() || 
        !initOnOffCluster() || !initLevelControlCluster()) {
        log_e("Failed to initialize clusters");
        return false;
    }

    // Create endpoint
    esp_zb_ep_list_t *ep = esp_zb_ep_list_create();
    if (!ep) {
        log_e("Failed to create endpoint");
        return false;
    }

    // Configure endpoint
    esp_zb_ep_config_t ep_config = {
        .endpoint = _endpoint,
        .app_device_id = ESP_ZB_HA_DIMMABLE_LIGHT_DEVICE_ID,
        .app_device_version = 0,
        .app_profile_id = ESP_ZB_AF_HA_PROFILE_ID,
        .cluster_list = _cluster_list
    };

    if (esp_zb_ep_list_add_ep(ep, &ep_config) != ESP_OK) {
        log_e("Failed to add endpoint configuration");
        esp_zb_ep_list_free(ep);
        return false;
    }

    _ep_list = ep;
    return true;
}

bool ZigbeeDimmableLight::initBasicCluster() {
    esp_zb_attribute_list_t *basic_cluster = esp_zb_basic_cluster_create(nullptr);
    if (!basic_cluster) {
        log_e("Failed to create basic cluster");
        return false;
    }
    
    if (esp_zb_cluster_list_add_basic_cluster(_cluster_list, basic_cluster, 
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE) != ESP_OK) {
        log_e("Failed to add basic cluster");
        return false;
    }
    
    return true;
}

bool ZigbeeDimmableLight::initOnOffCluster() {
    esp_zb_on_off_cluster_cfg_t on_off_cfg = {
        .on_off = _current_state
    };

    esp_zb_attribute_list_t *on_off_cluster = esp_zb_on_off_cluster_create(&on_off_cfg);
    if (!on_off_cluster) {
        log_e("Failed to create on/off cluster");
        return false;
    }

    if (esp_zb_cluster_list_add_on_off_cluster(_cluster_list, on_off_cluster,
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE) != ESP_OK) {
        log_e("Failed to add on/off cluster");
        return false;
    }

    return true;
}

bool ZigbeeDimmableLight::initPowerConfigCluster() {
    esp_zb_power_config_cluster_cfg_t power_cfg = {
        .voltage = (uint8_t)(_battery_voltage / 100),  // Convert to 100mV units
        .size = ESP_ZB_POWER_CONFIG_BATTERY_SIZE_OTHER,
        .quantity = 1,
        .remaining = 0
    };
    
    esp_zb_attribute_list_t *power_cluster = esp_zb_power_config_cluster_create(&power_cfg);
    if (!power_cluster) {
        log_e("Failed to create power config cluster");
        return false;
    }
    
    if (esp_zb_cluster_list_add_power_config_cluster(_cluster_list, power_cluster, 
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE) != ESP_OK) {
        log_e("Failed to add power config cluster");
        return false;
    }
    
    return true;
}

bool ZigbeeDimmableLight::initLevelControlCluster() {
    esp_zb_level_control_cluster_cfg_t level_cfg = {
        .current_level = _current_level
    };
    
    esp_zb_attribute_list_t *level_cluster = esp_zb_level_control_cluster_create(&level_cfg);
    if (!level_cluster) {
        log_e("Failed to create level control cluster");
        return false;
    }
    
    if (esp_zb_cluster_list_add_level_control_cluster(_cluster_list, level_cluster, 
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE) != ESP_OK) {
        log_e("Failed to add level control cluster");
        return false;
    }
    
    return true;
}

void ZigbeeDimmableLight::handleAttributeUpdate(esp_zb_zcl_set_attr_value_message_t *message) {
    if (!message || message->info.status != ESP_ZB_ZCL_STATUS_SUCCESS) {
        return;
    }

    uint16_t cluster_id = message->info.cluster;
    
    if (cluster_id == ESP_ZB_ZCL_CLUSTER_ID_ON_OFF) {
        if (message->attribute.id == ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID) {
            _current_state = *(bool*)message->attribute.data.value;
            if (_on_light_change) {
                _on_light_change(_current_state);
            }
        }
    } else if (cluster_id == ESP_ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL) {
        if (message->attribute.id == ESP_ZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_LEVEL_ID) {
            _current_level = *(uint8_t*)message->attribute.data.value;
            if (_on_level_change) {
                _on_level_change(_current_level);
            }
        }
    }
}

void ZigbeeDimmableLight::onLightChange(void (*callback)(bool)) {
    _on_light_change = callback;
}

void ZigbeeDimmableLight::onLevelChange(void (*callback)(uint8_t)) {
    _on_level_change = callback;
}

void ZigbeeDimmableLight::updateBatteryVoltage(uint32_t millivolts) {
    if (millivolts == _battery_voltage) {
        return;
    }
    
    _battery_voltage = millivolts;
    
    // Calculate percentage with bounds checking
    uint8_t percentage = 0;
    if (millivolts >= Config::Battery::MAX_MV) {
        percentage = 100;
    } else if (millivolts > Config::Battery::MIN_MV) {
        percentage = static_cast<uint8_t>(
            ((millivolts - Config::Battery::MIN_MV) * 100UL) / 
            (Config::Battery::MAX_MV - Config::Battery::MIN_MV)
        );
    }
    
    // Check if we should report low battery
    bool is_low_battery = percentage <= Config::Battery::LOW_THRESHOLD;
    if (is_low_battery != _low_battery_reported) {
        _low_battery_reported = is_low_battery;
        log_w("Battery low: %d%%", percentage);
    }
    
    // Only update if changed significantly
    if (abs(static_cast<int>(percentage) - static_cast<int>(_last_reported_percentage)) >= 
        Config::Battery::REPORT_CHANGE) {
        _last_reported_percentage = percentage;
        
        // Update voltage attribute (in 100mV units)
        uint8_t voltage = static_cast<uint8_t>(millivolts / 100);
        esp_err_t err = esp_zb_zcl_set_attribute_val(
            _endpoint,
            ESP_ZB_ZCL_CLUSTER_ID_POWER_CONFIG,
            ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
            ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_VOLTAGE_ID,
            &voltage,
            false);
            
        if (err != ESP_OK) {
            log_e("Failed to update battery voltage attribute: %s", esp_err_to_name(err));
            return;
        }
            
        // Update percentage attribute (in 0.5% units)
        uint8_t zigbee_percentage = percentage * 2;
        err = esp_zb_zcl_set_attribute_val(
            _endpoint,
            ESP_ZB_ZCL_CLUSTER_ID_POWER_CONFIG,
            ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
            ESP_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_PERCENTAGE_REMAINING_ID,
            &zigbee_percentage,
            false);
            
        if (err != ESP_OK) {
            log_e("Failed to update battery percentage attribute: %s", esp_err_to_name(err));
        }
    }
}