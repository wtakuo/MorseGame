#include "Settings.h"

#include <Preferences.h>
#include <string.h>

const char *keyModeName(KeyMode mode) {
    switch (mode) {
    case KeyMode::IambicA:
        return "IAMBIC-A";
    case KeyMode::IambicB:
        return "IAMBIC-B";
    default:
        return "STRAIGHT";
    }
}

const char *difficultyName(Difficulty difficulty) {
    switch (difficulty) {
    case Difficulty::Easy:
        return "EASY";
    case Difficulty::Hard:
        return "HARD";
    default:
        return "NORMAL";
    }
}

GameRules rulesFor(Difficulty difficulty) {
    GameRules r;  // the defaults are the normal rules
    switch (difficulty) {
    case Difficulty::Easy: {
        const uint32_t t[] = {8000, 12000, 16000, 20000, 24000};
        memcpy(r.timeoutMs, t, sizeof(t));
        r.successesPerLevel = 5;
        r.lives = 5;
        r.digitsInLevel1 = false;
        break;
    }
    case Difficulty::Hard: {
        const uint32_t t[] = {3000, 5000, 7000, 9000, 11000};
        memcpy(r.timeoutMs, t, sizeof(t));
        r.successesPerLevel = 10;
        r.lives = 3;
        r.digitsInLevel1 = true;
        break;
    }
    default:
        break;
    }
    return r;
}

namespace {

Preferences prefs;
int highScores[DIFFICULTY_COUNT];

const char *highScoreKey(Difficulty difficulty) {
    static const char *const keys[DIFFICULTY_COUNT] = {"hs0", "hs1", "hs2"};
    return keys[(int)difficulty];
}

}  // namespace

namespace Storage {

void begin() {
    prefs.begin("morsegame", false);
    for (int i = 0; i < DIFFICULTY_COUNT; i++) {
        highScores[i] = prefs.getInt(highScoreKey((Difficulty)i), 0);
    }
}

void loadSettings(Settings &s) {
    int mode = prefs.getUChar("keyMode", (uint8_t)s.keyMode);
    if (mode < KEY_MODE_COUNT) {
        s.keyMode = (KeyMode)mode;
    }
    int wpm = prefs.getUChar("wpm", s.wpm);
    if (wpm >= Settings::MIN_WPM && wpm <= Settings::MAX_WPM) {
        s.wpm = wpm;
    }
    s.swapPaddles = prefs.getUChar("swap", s.swapPaddles) != 0;
    int diff = prefs.getUChar("difficulty", (uint8_t)s.difficulty);
    if (diff < DIFFICULTY_COUNT) {
        s.difficulty = (Difficulty)diff;
    }
    int vol = prefs.getUChar("volume", s.volume);
    if (vol <= Settings::MAX_VOLUME) {
        s.volume = vol;
    }
}

// write only what changed, to spare the flash
static void putIfChanged(const char *key, uint8_t value) {
    if (!prefs.isKey(key) || prefs.getUChar(key) != value) {
        prefs.putUChar(key, value);
    }
}

void saveSettings(const Settings &s) {
    putIfChanged("keyMode", (uint8_t)s.keyMode);
    putIfChanged("wpm", s.wpm);
    putIfChanged("swap", s.swapPaddles);
    putIfChanged("difficulty", (uint8_t)s.difficulty);
    putIfChanged("volume", s.volume);
}

int highScore(Difficulty difficulty) {
    return highScores[(int)difficulty];
}

void setHighScore(Difficulty difficulty, int score) {
    highScores[(int)difficulty] = score;
    prefs.putInt(highScoreKey(difficulty), score);
}

void clearHighScores() {
    for (int i = 0; i < DIFFICULTY_COUNT; i++) {
        if (highScores[i] != 0) {
            setHighScore((Difficulty)i, 0);
        }
    }
}

}  // namespace Storage
