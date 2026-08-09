#ifndef TOUCH_HANDLER_H
#define TOUCH_HANDLER_H

#include <Arduino.h>
#include "config.h"

struct TouchPoint {
    int x;
    int y;
    uint32_t time;
    bool valid;
};

class TouchHandler {
private:
    TouchPoint previous_touch;
    TouchPoint current_touch;
    uint32_t last_touch_time;
    TouchMode current_mode;
    int touch_count;

public:
    TouchHandler();
    void init();
    void update();
    bool isTouched() const;
    TouchPoint getTouchPoint() const;
    TouchMode getMode() const { return current_mode; }
    bool isDoubleTap() const;
    int getTouchDelta(int& dx, int& dy);

private:
    bool readTouch(TouchPoint& point);
};

#endif
