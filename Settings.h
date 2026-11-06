// User settings, difficulty presets and high scores, kept in NVS.
#pragma once

#include <stdint.h>

#include "Game.h"

enum class KeyMode : uint8_t { Straight, IambicA, IambicB };
const int KEY_MODE_COUNT = 3;

enum class Difficulty : uint8_t { Easy, Normal, Hard };
const int DIFFICULTY_COUNT = 3;

struct Settings {
    static const int MIN_WPM = 5;
    static const int MAX_WPM = 40;
    static const int MAX_VOLUME = 10;

    KeyMode keyMode = KeyMode::Straight;
    int wpm = 15;               // keyer speed; initial speed for a straight key
    bool swapPaddles = false;   // false: pin 1 of the port = DIT, pin 2 = DAH
    Difficulty difficulty = Difficulty::Normal;
    int volume = 5;             // 0 - MAX_VOLUME

    // Length of a dot at the set speed (PARIS: 1200 / WPM ms).
    uint32_t dotMs() const { return 1200 / wpm; }
    uint8_t speakerVolume() const { return volume * 255 / MAX_VOLUME; }
};

const char *keyModeName(KeyMode mode);
const char *difficultyName(Difficulty difficulty);

// Game rules for each difficulty.
GameRules rulesFor(Difficulty difficulty);

// Persistent storage in NVS.
namespace Storage {

void begin();
void loadSettings(Settings &settings);   // invalid or missing values stay as they are
void saveSettings(const Settings &settings);
int highScore(Difficulty difficulty);
void setHighScore(Difficulty difficulty, int score);
void clearHighScores();

}  // namespace Storage
