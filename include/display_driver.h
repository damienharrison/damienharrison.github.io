#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include <LovyanGFX.hpp>
#include "config.h"

class DisplayDriver {
private:
    LGFX display;

public:
    DisplayDriver();
    void init();
    void drawRadarBackground();
    void drawRadarGrid(float radius_km);
    void drawPlane(int x, int y, int heading, bool selected = false, int altitude = 0);
    void drawText(int x, int y, const char* text, uint16_t color = COLOR_TEXT, int size = 1);
    void drawRect(int x, int y, int w, int h, uint16_t color);
    void drawFilledRect(int x, int y, int w, int h, uint16_t color);
    void drawCircle(int x, int y, int r, uint16_t color);
    void drawFilledCircle(int x, int y, int r, uint16_t color);
    void drawLine(int x1, int y1, int x2, int y2, uint16_t color);
    void clear();
    void setBacklight(uint8_t brightness);

    LGFX& getDisplay() { return display; }
};

#endif
