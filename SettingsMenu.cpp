#include "SettingsMenu.h"

#include <M5Unified.h>

#include "Sound.h"

namespace {

const int TOP = 40;          // first item
const int ROW = 22;          // item spacing
const int LABEL_X = 24;
const int VALUE_X = 150;
const int BUTTON_H = 36;     // labels (and, on a CoreS3, the touch area) at the bottom

const char *const ITEM_NAMES[] = {"KEY", "SPEED", "DIT / DAH", "DIFFICULTY", "VOLUME",
                                  "RESET", "EXIT"};

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
    resetDone_ = false;
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
        Sound::setVolume(s.speakerVolume());
        Sound::correct();  // let the player hear the new volume
        break;
    case RESET:
        s = Settings();
        Sound::setVolume(s.speakerVolume());
        resetDone_ = true;
        redrawAll_ = true;
        return;
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
        redrawAll_ = false;
    }
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
        M5.Display.print(resetDone_ ? "DONE" : "- / + : DEFAULTS");
        break;
    case EXIT:
        M5.Display.print("- / + : SAVE AND EXIT");
        break;
    }
}

void SettingsMenu::drawButtons() {
    int w = M5.Display.width(), h = M5.Display.height();
    const char *labels[] = {"NEXT", "-", "+"};
    M5.Display.setFont(&fonts::Font2);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(WHITE, BLACK);
    for (int i = 0; i < 3; i++) {
        int x = w * i / 3 + 6, bw = w / 3 - 12;
        M5.Display.drawRoundRect(x, h - BUTTON_H + 4, bw, BUTTON_H - 6, 6, DARKGREY);
        int tw = M5.Display.textWidth(labels[i]);
        M5.Display.setCursor(x + (bw - tw) / 2, h - BUTTON_H / 2 - 7);
        M5.Display.print(labels[i]);
    }
}
