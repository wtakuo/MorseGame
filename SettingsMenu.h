// Settings screen, operated with the three buttons:
//   left = next item, centre = previous value (-), right = next value (+).
// On a CoreS3 the buttons are the bottom of the touch screen, where their
// labels are drawn.
#pragma once

#include "Settings.h"

class SettingsMenu {
public:
    enum class Button { Next, Minus, Plus };

    // Show the menu for editing a copy of the settings.
    void open(const Settings &settings);
    // Handle a button press. Returns true when the menu has been closed;
    // settings() then holds the result.
    bool press(Button button);
    // Whether holding the button should repeat it (for numbers).
    bool repeats() const;

    const Settings &settings() const { return settings_; }

    // Redraw what changed; call every loop while the menu is open.
    void update();

private:
    enum Item { KEY, SPEED, PADDLE, DIFFICULTY, VOLUME, RESET, EXIT, ITEM_COUNT };

    void change(int step);
    void drawItem(int item);
    void drawButtons();

    Settings settings_;
    int cursor_ = KEY;
    bool redrawAll_ = true;
    bool itemChanged_[ITEM_COUNT] = {};
    bool resetDone_ = false;
};
