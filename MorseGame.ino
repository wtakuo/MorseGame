// Morse Game for M5Stack
//
//   Config.h         pins and loop timing
//   MorseCode.*      Morse code table
//   MorseDecoder.*   straight key debounce, speed tracking, characters
//   Game.*           game state, judgement, score and levels
//   Words.*          challenge words
//   Screen.*         drawing
//   Sound.*          sidetone and sound effects

#include <M5Unified.h>

#include "Config.h"
#include "Game.h"
#include "MorseDecoder.h"
#include "Screen.h"
#include "Sound.h"

const float INITIAL_DOT_MS = 80;  // about 15 WPM
const uint8_t VOLUME = 220;

GameRules rules;
Game game(rules);
Screen screen;
StraightKey key;
SpeedTracker speed(INITIAL_DOT_MS);
CharacterDecoder decoder;

uint32_t lastIterTime;

// Sample the key, decode it and advance the game.
void iter(uint32_t now) {
    if (key.update(now, digitalRead(KEY_PIN) == LOW)) {
        if (key.isDown()) {
            Sound::sidetoneOn();
        }
        else {
            decoder.addMark(speed.classify(key.lastMarkLength()));
            Sound::sidetoneOff();
        }
        screen.setKeyLamp(key.isDown());
    }
    if (!key.isDown()) {
        uint32_t gap = now - key.lastChangeTime();
        if (const char *text = decoder.checkCharacterEnd(gap, speed.charGap())) {
            game.addCharacter(text);
            screen.appendInput(text);
        }
        if (decoder.checkWordEnd(gap, speed.wordGap())) {
            game.endWord();
            screen.appendInput(" ");
        }
    }
    game.update(now);
}

void setup() {
    auto cfg = M5.config();
    cfg.internal_spk = true;
    cfg.serial_baudrate = 115200;
    M5.begin(cfg);
    pinMode(KEY_PIN, INPUT_PULLUP);
    screen.begin();
    Sound::begin(VOLUME);
    lastIterTime = millis();
}

void loop() {
    uint32_t t = millis();
    M5.update();
    if (M5.BtnB.wasPressed()) {
        game.start();
    }
    if (t - lastIterTime >= ITER_INTERVAL_MS) {
        iter(t);
        lastIterTime = t;
    }
    screen.update(game);
}
