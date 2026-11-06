#include "SettingsMenu.h"

#include <M5Unified.h>

#include "Sound.h"

namespace {

const int TOP = 36;          // first item
const int ROW = 20;          // item spacing
const int LABEL_X = 24;
const int VALUE_X = 150;
const int BUTTON_H = 36;     // labels (and, on a CoreS3, the touch area) at the bottom

const char *const ITEM_NAMES[] = {"KEY", "SPEED", "DIT / DAH", "LEVEL", "VOLUME",
                                  "RESET", "CLEAR SCORES", "EXIT"};

bool isCoreS3() {
    auto b = M5.getBoard();
    return b == m5::board_t::board_M5StackCoreS3 || b == m5::board_t::board_M5StackCoreS3SE;
}

template <typename E>
E cycle(E value, int count, int step) {
    return (E)(((int)value + step + count) % count);
}

}  // namespace

void SettingsMenu::open(const Settings &settings) {
    settings_ = settings;
    cursor_ = KEY;
    armed_ = done_ = -1;
    battery_ = -2;
    nextBatteryCheck_ = 0;
    redrawAll_ = true;
    if (isCoreS3()) {
        // let the button labels at the bottom of the screen act as BtnA-C
        M5.setTouchButtonHeight(BUTTON_H);
    }
}

bool SettingsMenu::repeats() const {
    return cursor_ == SPEED || cursor_ == VOLUME;
}

bool SettingsMenu::press(Button button) {
    if (button == Button::Next) {
        int old = cursor_;
        cursor_ = (cursor_ + 1) % ITEM_COUNT;
        armed_ = done_ = -1;   // moving away cancels a confirmation
        itemChanged_[old] = itemChanged_[cursor_] = true;
        return false;
    }
    int step = button == Button::Plus ? 1 : -1;
    if (cursor_ == EXIT) {
        if (isCoreS3()) {
            M5.setTouchButtonHeight(0);
        }
        return true;
    }
    change(step);
    return false;
}

void SettingsMenu::change(int step) {
    Settings &s = settings_;
    switch (cursor_) {
    case KEY:
        s.keyMode = cycle(s.keyMode, KEY_MODE_COUNT, step);
        break;
    case SPEED:
        s.wpm = constrain(s.wpm + step, Settings::MIN_WPM, Settings::MAX_WPM);
        break;
    case PADDLE:
        s.swapPaddles = !s.swapPaddles;
        break;
    case DIFFICULTY:
        s.difficulty = cycle(s.difficulty, DIFFICULTY_COUNT, step);
        break;
    case VOLUME:
        s.volume = constrain(s.volume + step, 0, Settings::MAX_VOLUME);
        // the click of the button (played after the change) has the new volume
        Sound::setVolume(s.speakerVolume());
        break;
    case RESET:
    case CLEAR_SCORES:
        if (armed_ != cursor_) {
            armed_ = cursor_;   // ask for a second press
            done_ = -1;
            break;
        }
        armed_ = -1;
        done_ = cursor_;
        if (cursor_ == RESET) {
            s = Settings();
            Sound::setVolume(s.speakerVolume());
            redrawAll_ = true;
            return;
        }
        Storage::clearHighScores();
        break;
    default:
        return;
    }
    itemChanged_[cursor_] = true;
}

void SettingsMenu::update() {
    M5.Display.startWrite();
    if (redrawAll_) {
        M5.Display.fillScreen(BLACK);
        M5.Display.setFont(&fonts::Font2);
        M5.Display.setTextSize(1);
        M5.Display.setTextColor(CYAN, BLACK);
        M5.Display.setCursor(LABEL_X, 12);
        M5.Display.print("SETTINGS");
        for (int i = 0; i < ITEM_COUNT; i++) {
            itemChanged_[i] = true;
        }
        drawButtons();
        battery_ = -2;   // draw the battery level again
        redrawAll_ = false;
    }
    drawBattery();
    for (int i = 0; i < ITEM_COUNT; i++) {
        if (itemChanged_[i]) {
            drawItem(i);
            itemChanged_[i] = false;
        }
    }
    M5.Display.endWrite();
}

void SettingsMenu::drawItem(int item) {
    const Settings &s = settings_;
    int y = TOP + item * ROW;
    bool selected = item == cursor_;
    M5.Display.fillRect(0, y - 2, M5.Display.width(), ROW - 2, selected ? NAVY : BLACK);
    uint16_t bg = selected ? NAVY : BLACK;
    M5.Display.setFont(&fonts::Font2);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(selected ? YELLOW : WHITE, bg);
    M5.Display.setCursor(8, y);
    M5.Display.print(selected ? ">" : " ");
    M5.Display.setCursor(LABEL_X, y);
    M5.Display.print(ITEM_NAMES[item]);
    M5.Display.setTextColor(selected ? YELLOW : GREEN, bg);
    M5.Display.setCursor(VALUE_X, y);
    switch (item) {
    case KEY:
        M5.Display.print(keyModeName(s.keyMode));
        break;
    case SPEED:
        M5.Display.printf("%d WPM", s.wpm);
        break;
    case PADDLE:
        M5.Display.print(s.swapPaddles ? "REVERSED" : "NORMAL");
        break;
    case DIFFICULTY:
        M5.Display.print(difficultyName(s.difficulty));
        break;
    case VOLUME:
        M5.Display.printf("%d", s.volume);
        break;
    case RESET:
    case CLEAR_SCORES:
        if (armed_ == item) {
            M5.Display.setTextColor(RED, bg);
            M5.Display.print("PRESS AGAIN");
        }
        else if (done_ == item) {
            M5.Display.print("DONE");
        }
        else {
            M5.Display.print(item == RESET ? "TO DEFAULTS" : "ALL LEVELS");
        }
        break;
    case EXIT:
        M5.Display.print("SAVE");
        break;
    }
}

// Battery level at the top right, "+" in front while charging. Checked
// once a second and drawn only when it changes.
void SettingsMenu::drawBattery() {
    uint32_t now = millis();
    if (battery_ != -2 && now < nextBatteryCheck_) {
        return;
    }
    nextBatteryCheck_ = now + 1000;
    int level = M5.Power.getBatteryLevel();
    bool charging = M5.Power.isCharging() == m5::Power_Class::is_charging;
    if (level == battery_ && charging == charging_) {
        return;
    }
    battery_ = level;
    charging_ = charging;
    char text[8];
    if (level < 0 || level > 100) {
        snprintf(text, sizeof(text), "--%%");
    }
    else {
        snprintf(text, sizeof(text), "%s%d%%", charging ? "+" : "", level);
    }
    M5.Display.setFont(&fonts::Font2);
    M5.Display.setTextSize(1);
    int w = M5.Display.width();
    M5.Display.fillRect(w - 60, 12, 60, M5.Display.fontHeight(M5.Display.getFont()), BLACK);
    M5.Display.setTextColor(charging ? GREEN : WHITE, BLACK);
    M5.Display.setCursor(w - 8 - M5.Display.textWidth(text), 12);
    M5.Display.print(text);
}

void SettingsMenu::drawButtons() {
    int w = M5.Display.width(), h = M5.Display.height();
    M5.Display.setFont(&fonts::Font2);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(WHITE, BLACK);
    int cy = h - BUTTON_H / 2 + 1;   // centre line of the buttons
    for (int i = 0; i < 3; i++) {
        int x = w * i / 3 + 6, bw = w / 3 - 12, cx = x + bw / 2;
        M5.Display.drawRoundRect(x, h - BUTTON_H + 4, bw, BUTTON_H - 6, 6, DARKGREY);
        if (i == 0) {        // down-pointing triangle: next item
            M5.Display.fillTriangle(cx, cy + 7, cx - 8, cy - 7, cx + 8, cy - 7, WHITE);
        }
        else if (i == 1) {   // left-pointing triangle
            M5.Display.fillTriangle(cx - 8, cy, cx + 6, cy - 8, cx + 6, cy + 8, WHITE);
        }
        else {               // right-pointing triangle
            M5.Display.fillTriangle(cx + 8, cy, cx - 6, cy - 8, cx - 6, cy + 8, WHITE);
        }
    }
}
