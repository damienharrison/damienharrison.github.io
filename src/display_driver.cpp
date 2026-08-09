#include "display_driver.h"

DisplayDriver::DisplayDriver() {
    auto cfg = display.config();

    // GC9A01 round display configuration
    cfg.panel_height = 240;
    cfg.panel_width = 240;

    // SPI Configuration for ESP32-S3
    cfg.spi_host = SPI2_HOST;
    cfg.spi_mode = 0;
    cfg.freq_write = 80000000;
    cfg.freq_read = 16000000;
    cfg.spi_3wire = false;

    // Pin configuration - adjust to your wiring
    cfg.pin_cs = 10;
    cfg.pin_mosi = 11;
    cfg.pin_miso = 13;
    cfg.pin_sclk = 12;
    cfg.pin_dc = 8;
    cfg.pin_rst = 9;
    cfg.pin_busy = -1;
    cfg.pin_backlight = 46;

    cfg.backlight_high = true;
    cfg.invert = true;
    cfg.rgb_order = true;
    cfg.dlen_16bit = false;
    cfg.bus_shared = true;
    cfg.memory_width = 240;
    cfg.memory_height = 240;
    cfg.panel_width = 240;
    cfg.panel_height = 240;
    cfg.offset_x = 0;
    cfg.offset_y = 0;

    // Touch panel configuration
    cfg.touch_height = 240;
    cfg.touch_width = 240;
    cfg.touch_pin_int = TOUCH_INT;
    cfg.touch_pin_rst = -1;
    cfg.i2c_port = I2C_NUM_0;
    cfg.i2c_addr = 0x5D;
    cfg.pin_sda = 3;
    cfg.pin_scl = 46;
    cfg.freq = 100000;

    display.config(cfg);
}

void DisplayDriver::init() {
    display.init();
    display.startWrite();
    display.fillScreen(COLOR_BLACK);
    display.endWrite();
    setBacklight(255);
}

void DisplayDriver::drawRadarBackground() {
    display.startWrite();
    // Draw circular radar background
    display.fillCircle(CENTER_X, CENTER_Y, RADIUS, COLOR_RADAR_BG);
    // Draw outer circle border
    display.drawCircle(CENTER_X, CENTER_Y, RADIUS, COLOR_RADAR_GRID);
    display.endWrite();
}

void DisplayDriver::drawRadarGrid(float radius_km) {
    display.startWrite();

    // Draw concentric circles at 1/4, 1/2, 3/4 radius
    float step = RADIUS / 4.0f;
    for (int i = 1; i < 4; i++) {
        int r = (int)(step * i);
        display.drawCircle(CENTER_X, CENTER_Y, r, COLOR_RADAR_GRID);
    }

    // Draw cardinal directions (N, S, E, W)
    display.drawLine(CENTER_X, CENTER_Y - RADIUS, CENTER_X, CENTER_Y - RADIUS - 5, COLOR_TEXT);
    display.drawLine(CENTER_X, CENTER_Y + RADIUS, CENTER_X, CENTER_Y + RADIUS + 5, COLOR_TEXT);
    display.drawLine(CENTER_X - RADIUS, CENTER_Y, CENTER_X - RADIUS - 5, CENTER_Y, COLOR_TEXT);
    display.drawLine(CENTER_X + RADIUS, CENTER_Y, CENTER_X + RADIUS + 5, CENTER_Y, COLOR_TEXT);

    // Draw radial lines at 45-degree intervals
    for (int angle = 0; angle < 360; angle += 45) {
        float rad = angle * M_PI / 180.0f;
        int x1 = CENTER_X + RADIUS * cos(rad);
        int y1 = CENTER_Y + RADIUS * sin(rad);
        display.drawLine(CENTER_X, CENTER_Y, x1, y1, COLOR_RADAR_GRID);
    }

    display.endWrite();
}

void DisplayDriver::drawPlane(int x, int y, int heading, bool selected, int altitude) {
    display.startWrite();

    // Select color based on altitude
    uint16_t color = COLOR_PLANE_ACTIVE;
    if (altitude > ALTITUDE_HIGH) {
        color = COLOR_ALTITUDE_HIGH;
    } else if (altitude > ALTITUDE_MID) {
        color = COLOR_ALTITUDE_MID;
    } else {
        color = COLOR_ALTITUDE_LOW;
    }

    if (selected) {
        color = COLOR_PLANE_SELECTED;
    }

    // Draw plane as a triangle pointing in heading direction
    float rad = heading * M_PI / 180.0f;
    int size = 6;

    int px1 = x + size * cos(rad);
    int py1 = y + size * sin(rad);

    int px2 = x + size * cos(rad + 2.4f);
    int py2 = y + size * sin(rad + 2.4f);

    int px3 = x + size * cos(rad - 2.4f);
    int py3 = y + size * sin(rad - 2.4f);

    display.fillTriangle(px1, py1, px2, py2, px3, py3, color);

    if (selected) {
        display.drawCircle(x, y, size + 3, COLOR_PLANE_SELECTED);
    }

    display.endWrite();
}

void DisplayDriver::drawText(int x, int y, const char* text, uint16_t color, int size) {
    display.startWrite();
    display.setTextColor(color);
    display.setTextSize(size);
    display.setCursor(x, y);
    display.print(text);
    display.endWrite();
}

void DisplayDriver::drawRect(int x, int y, int w, int h, uint16_t color) {
    display.startWrite();
    display.drawRect(x, y, w, h, color);
    display.endWrite();
}

void DisplayDriver::drawFilledRect(int x, int y, int w, int h, uint16_t color) {
    display.startWrite();
    display.fillRect(x, y, w, h, color);
    display.endWrite();
}

void DisplayDriver::drawCircle(int x, int y, int r, uint16_t color) {
    display.startWrite();
    display.drawCircle(x, y, r, color);
    display.endWrite();
}

void DisplayDriver::drawFilledCircle(int x, int y, int r, uint16_t color) {
    display.startWrite();
    display.fillCircle(x, y, r, color);
    display.endWrite();
}

void DisplayDriver::drawLine(int x1, int y1, int x2, int y2, uint16_t color) {
    display.startWrite();
    display.drawLine(x1, y1, x2, y2, color);
    display.endWrite();
}

void DisplayDriver::clear() {
    display.startWrite();
    display.fillScreen(COLOR_BLACK);
    display.endWrite();
}

void DisplayDriver::setBacklight(uint8_t brightness) {
    display.setBrightness(brightness);
}
