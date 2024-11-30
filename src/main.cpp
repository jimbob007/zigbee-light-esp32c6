#include "globals.h"
#include "Button.h"

// Global button instance
Button button;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\nInitializing XIAO ESP32C6 Zigbee Light...");

    // Check wake reason
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    Serial.printf("Wake reason: %d\n", wakeup_reason);

    if (!Functions::initHardware()) {
        Serial.println("Hardware initialization failed!");
        ESP.restart();
    }

    // Initialize Zigbee
    Globals::zbLight = new ZigbeeDimmableLight(Config::Zigbee::ENDPOINT);
    if (!Globals::zbLight || !Globals::zbLight->begin()) {
        Serial.println("Failed to initialize Zigbee endpoint!");
        ESP.restart();
    }

    // Setup callbacks
    Globals::zbLight->onLightChange(Functions::setLED);
    Globals::zbLight->onLevelChange(Functions::setLEDLevel);
    esp_zb_zcl_set_attr_value_message_cb(Functions::zbAttributeChangedCallback);
    esp_zb_zcl_status_message_cb(Functions::zbStatusCallback);

    // Start Zigbee stack
    Serial.println("Starting Zigbee stack...");
    esp_zb_cfg_t zb_cfg = ESP_ZB_DEFAULT_CONFIG();
    zb_cfg.role = ESP_ZB_ZR_ROLE;  // Router role for better network stability
    zb_cfg.endpoint_list = Globals::zbLight->getEndpointList();

    if (esp_zb_start(&zb_cfg) != ESP_OK) {
        Serial.println("Failed to start Zigbee stack!");
        ESP.restart();
    }

    Functions::checkVoltage();
    Globals::last_activity = millis();
}

void loop() {
    static bool factory_reset_pending = false;
    static bool joining = false;
    uint32_t now = millis();

    // Update button state
    button.update();
    
    // Handle button press
    if (button.isPressed()) {
        Globals::last_activity = now;
        
        // Check for factory reset
        if (button.getPressTime() >= Config::Sleep::FACTORY_RESET_TIME_MS) {
            if (!factory_reset_pending && Functions::confirmFactoryReset()) {
                Serial.println("Factory reset confirmed");
                esp_zb_factory_reset();
            }
            factory_reset_pending = true;
        }
    } else {
        if (factory_reset_pending) {
            factory_reset_pending = false;
        } else if (button.wasPressed() && button.getPressTime() > 0) {
            Functions::toggleLight();
        }
    }

    // Periodic tasks
    if (now - Globals::last_voltage_check >= Config::Battery::CHECK_INTERVAL_MS) {
        Functions::checkVoltage();
        Globals::last_voltage_check = now;
    }

    // Sleep check
    if (!factory_reset_pending && !joining && 
        (now - Globals::last_activity) >= Config::Sleep::DELAY_MS) {
        Functions::checkAndEnterSleep();
    }

    // Handle Zigbee stack events
    esp_zb_main_loop_iteration();
    delay(10);  // Small delay to prevent watchdog issues
}