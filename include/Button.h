#pragma once

#include <Arduino.h>
#include "config.h"

class Button {
public:
    Button() : _last_state(false), _current_state(false), _press_time(0), _was_pressed(false) {}

    void update() {
        bool new_state = digitalRead(Config::Pins::BUTTON) == HIGH;
        
        if (new_state != _current_state) {
            if (new_state) {
                _press_time = millis();
                _was_pressed = false;
            } else {
                _was_pressed = true;
            }
            _current_state = new_state;
        }
        
        _last_state = _current_state;
    }

    bool isPressed() const {
        return _current_state;
    }

    bool wasPressed() const {
        return _was_pressed;
    }

    uint32_t getPressTime() const {
        if (_current_state) {
            return millis() - _press_time;
        }
        return 0;
    }

private:
    bool _last_state;
    bool _current_state;
    uint32_t _press_time;
    bool _was_pressed;
};