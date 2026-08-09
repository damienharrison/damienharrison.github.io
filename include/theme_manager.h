#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include "config.h"
#include <SPIFFS.h>

class ThemeManager {
private:
    ThemeMode current_theme;
    const char* THEME_FILE = "/spiffs/theme.json";

public:
    ThemeManager();
    void init();
    void loadTheme();
    void saveTheme();
    ThemeMode getTheme() const { return current_theme; }
    void setTheme(ThemeMode theme);
    void toggleTheme();
    void applyTheme();

private:
    void applyLightTheme();
    void applyDarkTheme();
};

#endif
