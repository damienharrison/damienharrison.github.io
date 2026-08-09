#ifndef VIRTUAL_KEYBOARD_H
#define VIRTUAL_KEYBOARD_H

#include <Arduino.h>
#include "display_driver.h"
#include "touch_handler.h"

class VirtualKeyboard {
private:
    static constexpr int KEYBOARD_WIDTH = 220;
    static constexpr int KEYBOARD_HEIGHT = 140;
    static constexpr int KEY_WIDTH = 20;
    static constexpr int KEY_HEIGHT = 14;
    static constexpr int KEYBOARD_X = 10;
    static constexpr int KEYBOARD_Y = 100;

    String input_text;
    String keyboard_layout = "qwertyuiopasdfghjklzxcvbnm.";
    int selected_key_idx = -1;
    bool shift_active = false;

public:
    VirtualKeyboard();
    void draw(DisplayDriver& display);
    void handleTouch(int touch_x, int touch_y);
    String getText() const { return input_text; }
    void setText(const String& text) { input_text = text; }
    void clear() { input_text = ""; selected_key_idx = -1; }
    void backspace();
    void addChar(char c);
    bool handleInput(TouchPoint touch);

private:
    void drawKeyboardKey(DisplayDriver& display, int idx, bool highlighted);
    int getTouchKeyIndex(int x, int y) const;
    char getCharAtIndex(int idx);
};

#endif
