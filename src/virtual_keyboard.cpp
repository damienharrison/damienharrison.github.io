#include "virtual_keyboard.h"

VirtualKeyboard::VirtualKeyboard() {}

void VirtualKeyboard::draw(DisplayDriver& display) {
    // Draw input box at top
    display.drawFilledRect(5, 10, 230, 25, COLOR_BUTTON);
    display.drawRect(5, 10, 230, 25, COLOR_TEXT);
    display.drawText(10, 15, input_text.c_str(), COLOR_TEXT, 1);

    // Draw keyboard background
    display.drawFilledRect(KEYBOARD_X, KEYBOARD_Y, KEYBOARD_WIDTH, KEYBOARD_HEIGHT, COLOR_RADAR_BG);
    display.drawRect(KEYBOARD_X, KEYBOARD_Y, KEYBOARD_WIDTH, KEYBOARD_HEIGHT, COLOR_RADAR_GRID);

    // Draw all keys
    int cols = 10;
    for (int i = 0; i < (int)keyboard_layout.length(); i++) {
        drawKeyboardKey(display, i, i == selected_key_idx);
    }

    // Draw backspace key
    int bs_x = KEYBOARD_X + 200;
    int bs_y = KEYBOARD_Y + 100;
    display.drawFilledRect(bs_x, bs_y, 18, 12, COLOR_BUTTON);
    display.drawRect(bs_x, bs_y, 18, 12, COLOR_TEXT);
    display.drawText(bs_x + 2, bs_y + 2, "<", COLOR_TEXT, 1);

    // Draw space key
    int space_x = KEYBOARD_X + 10;
    int space_y = KEYBOARD_Y + 100;
    display.drawFilledRect(space_x, space_y, 120, 12, COLOR_BUTTON);
    display.drawRect(space_x, space_y, 120, 12, COLOR_TEXT);
    display.drawText(space_x + 5, space_y + 2, "Space", COLOR_TEXT, 1);
}

void VirtualKeyboard::drawKeyboardKey(DisplayDriver& display, int idx, bool highlighted) {
    int cols = 10;
    int row = idx / cols;
    int col = idx % cols;

    int x = KEYBOARD_X + col * KEY_WIDTH;
    int y = KEYBOARD_Y + row * KEY_HEIGHT;

    uint16_t bg_color = highlighted ? COLOR_BUTTON_ACTIVE : COLOR_BUTTON;
    uint16_t text_color = COLOR_TEXT;

    display.drawFilledRect(x, y, KEY_WIDTH - 1, KEY_HEIGHT - 1, bg_color);
    display.drawRect(x, y, KEY_WIDTH - 1, KEY_HEIGHT - 1, text_color);

    char c = keyboard_layout[idx];
    char key_char[] = {c, '\0'};
    if (shift_active && c >= 'a' && c <= 'z') {
        key_char[0] = c - 32;  // Convert to uppercase
    }
    display.drawText(x + 3, y + 2, key_char, text_color, 1);
}

int VirtualKeyboard::getTouchKeyIndex(int x, int y) const {
    if (x < KEYBOARD_X || x > KEYBOARD_X + KEYBOARD_WIDTH ||
        y < KEYBOARD_Y || y > KEYBOARD_Y + KEYBOARD_HEIGHT) {
        return -1;
    }

    int rel_x = x - KEYBOARD_X;
    int rel_y = y - KEYBOARD_Y;

    int col = rel_x / KEY_WIDTH;
    int row = rel_y / KEY_HEIGHT;

    if (col >= 10 || row >= 3) {
        return -1;
    }

    return row * 10 + col;
}

char VirtualKeyboard::getCharAtIndex(int idx) {
    if (idx >= 0 && idx < (int)keyboard_layout.length()) {
        return keyboard_layout[idx];
    }
    return '\0';
}

void VirtualKeyboard::backspace() {
    if (input_text.length() > 0) {
        input_text = input_text.substring(0, input_text.length() - 1);
    }
}

void VirtualKeyboard::addChar(char c) {
    if (input_text.length() < 32) {  // Max 32 chars for SSID/password
        input_text += c;
    }
}

bool VirtualKeyboard::handleInput(TouchPoint touch) {
    // Check backspace key
    if (touch.x > KEYBOARD_X + 200 && touch.x < KEYBOARD_X + 218 &&
        touch.y > KEYBOARD_Y + 100 && touch.y < KEYBOARD_Y + 112) {
        backspace();
        return true;
    }

    // Check space key
    if (touch.x > KEYBOARD_X + 10 && touch.x < KEYBOARD_X + 130 &&
        touch.y > KEYBOARD_Y + 100 && touch.y < KEYBOARD_Y + 112) {
        addChar(' ');
        return true;
    }

    // Check regular keys
    int key_idx = getTouchKeyIndex(touch.x, touch.y);
    if (key_idx >= 0) {
        char c = getCharAtIndex(key_idx);
        if (shift_active && c >= 'a' && c <= 'z') {
            addChar(c - 32);  // Add uppercase
        } else {
            addChar(c);
        }
        selected_key_idx = key_idx;
        return true;
    }

    return false;
}
