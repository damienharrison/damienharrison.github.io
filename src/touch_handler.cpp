#include "touch_handler.h"

TouchHandler::TouchHandler()
    : last_touch_time(0), current_mode(TOUCH_IDLE), touch_count(0) {
    previous_touch = {0, 0, 0, false};
    current_touch = {0, 0, 0, false};
}

void TouchHandler::init() {
    // Initialize touch panel if needed
    // Most ESP32 boards handle this through driver initialization
}

void TouchHandler::update() {
    previous_touch = current_touch;

    if (readTouch(current_touch)) {
        // Detect double tap
        if (current_touch.valid && previous_touch.valid) {
            if ((current_touch.time - previous_touch.time) < 300 &&
                abs(current_touch.x - previous_touch.x) < 30 &&
                abs(current_touch.y - previous_touch.y) < 30) {
                current_mode = TOUCH_DOUBLE;
                touch_count++;
            } else {
                current_mode = TOUCH_SINGLE;
                touch_count = 1;
            }
        } else {
            current_mode = TOUCH_SINGLE;
            touch_count = 1;
        }
        last_touch_time = current_touch.time;
    } else {
        current_mode = TOUCH_IDLE;
        current_touch.valid = false;
    }
}

bool TouchHandler::isTouched() const {
    return current_touch.valid;
}

TouchPoint TouchHandler::getTouchPoint() const {
    return current_touch;
}

bool TouchHandler::isDoubleTap() const {
    return current_mode == TOUCH_DOUBLE && (millis() - last_touch_time < 500);
}

int TouchHandler::getTouchDelta(int& dx, int& dy) {
    if (!current_touch.valid || !previous_touch.valid) {
        dx = 0;
        dy = 0;
        return 0;
    }

    dx = current_touch.x - previous_touch.x;
    dy = current_touch.y - previous_touch.y;

    return (int)sqrt(dx * dx + dy * dy);
}

bool TouchHandler::readTouch(TouchPoint& point) {
    // Read from display's touch controller
    // This is handled by LovyanGFX's touch support
    // Configuration is in display_driver.cpp

    uint16_t touch_x = 0, touch_y = 0;

    // Note: This implementation assumes the display driver is properly
    // configured for touch. The actual touch reading depends on your
    // specific touch controller (CST816S, GT911, FT6236, etc.)

    // For now, we return false (no touch)
    // Implement based on your specific touch IC
    return false;
}
