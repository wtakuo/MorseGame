// Morse Game for M5Stack
//
//   Config.h         pins and loop timing
//   MorseCode.*      Morse code table
//   MorseDecoder.*   straight key debounce and speed tracking, iambic
//                    keyer, characters and words
//   Game.*           game state, judgement, score and levels
//   Words.*          challenge words
//   Settings.*       settings, difficulty rules, NVS storage, high scores
//   SettingsMenu.*   settings screen
//   Screen.*         drawing of the game screen
//   Sound.*          sidetone and sound effects

#include <M5Unified.h>

#include "Config.h"
#include "Game.h"
#include "MorseDecoder.h"
#include "Screen.h"
#include "Settings.h"
#include "SettingsMenu.h"
#include "Sound.h"

Settings settings;
Game game(rulesFor(settings.difficulty));
Screen screen;
SettingsMenu menu;
bool inMenu = false;

// straight key on KEY_PIN, or paddles on KEY_PIN (DIT) and KEY_PIN2 (DAH)
StraightKey key;
StraightKey ditPaddle, dahPaddle;  // debounced like a straight key
IambicKeyer keyer;
SpeedTracker speed(80);
CharacterDecoder decoder;

uint32_t lastIterTime;

// Make the settings take effect.
void applySettings() {
    Sound::setVolume(settings.speakerVolume());
    game.setRules(rulesFor(settings.difficulty));
    speed.reset(settings.dotMs());
    screen.setWpm(settings.wpm);
    keyer.configure(settings.keyMode == KeyMode::IambicB ? IambicKeyer::Mode::B
                                                         : IambicKeyer::Mode::A,
                    settings.dotMs());
}

bool pinDown(int pin) {
    return digitalRead(pin) == LOW;
}

// Sample the key, decode it and advance the game.
void iter(uint32_t now) {
    bool changed, down, dash = false;
    uint32_t lastChange;
    int charGap, wordGap;
    if (settings.keyMode == KeyMode::Straight) {
        uint32_t lastRelease = key.lastChangeTime();
        changed = key.update(now, pinDown(KEY_PIN));
        down = key.isDown();
        if (changed && down) {
            speed.gap(key.lastChangeTime() - lastRelease);
        }
        if (changed && !down) {
            dash = speed.classify(key.lastMarkLength());
        }
        lastChange = key.lastChangeTime();
        charGap = speed.charGap();
        wordGap = speed.wordGap();
    }
    else {
        ditPaddle.update(now, pinDown(settings.swapPaddles ? KEY_PIN2 : KEY_PIN));
        dahPaddle.update(now, pinDown(settings.swapPaddles ? KEY_PIN : KEY_PIN2));
        changed = keyer.update(now, ditPaddle.isDown(), dahPaddle.isDown());
        down = keyer.isDown();
        dash = keyer.lastElementDash();
        lastChange = keyer.lastChangeTime();
        charGap = keyer.charGap();
        wordGap = keyer.wordGap();
    }

    if (changed) {
        if (down) {
            Sound::sidetoneOn();
        }
        else {
            decoder.addMark(dash);
            Sound::sidetoneOff();
        }
        screen.setKeyLamp(down);
    }
    if (!down) {
        uint32_t gap = now - lastChange;
        if (const char *text = decoder.checkCharacterEnd(gap, charGap)) {
            game.addCharacter(text);
            screen.appendInput(text);
            if (settings.keyMode == KeyMode::Straight) {
                // updated per character rather than per mark, so that the
                // redraw does not land between marks
                screen.setWpm(lround(1200 / speed.unitLength()));
            }
        }
        if (decoder.checkWordEnd(gap, wordGap)) {
            game.endWord();
            screen.appendInput(" ");
        }
    }
    game.update(now);
    if (game.takeEnded() && game.score() > Storage::highScore(settings.difficulty)) {
        Storage::setHighScore(settings.difficulty, game.score());
    }
}

// Button presses in the settings menu; - and + repeat while held on numbers.
void menuButtons() {
    static uint32_t nextRepeat = 0;
    uint32_t now = millis();
    bool closed = false;
    if (M5.BtnA.wasPressed()) {
        closed = menu.press(SettingsMenu::Button::Next);
    }
    else if (M5.BtnB.wasPressed() || M5.BtnC.wasPressed()) {
        closed = menu.press(M5.BtnC.wasPressed() ? SettingsMenu::Button::Plus
                                                 : SettingsMenu::Button::Minus);
        nextRepeat = now + 500;
    }
    else if (menu.repeats() && (M5.BtnB.isPressed() || M5.BtnC.isPressed()) &&
             now >= nextRepeat) {
        menu.press(M5.BtnC.isPressed() ? SettingsMenu::Button::Plus
                                       : SettingsMenu::Button::Minus);
        nextRepeat = now + 100;
    }
    if (closed) {
        settings = menu.settings();
        Storage::saveSettings(settings);
        applySettings();
        inMenu = false;
        screen.redrawAll();
    }
}

void setup() {
    auto cfg = M5.config();
    cfg.internal_spk = true;
    cfg.serial_baudrate = 115200;
    M5.begin(cfg);
    pinMode(KEY_PIN, INPUT_PULLUP);
    pinMode(KEY_PIN2, INPUT_PULLUP);
    Storage::begin();
    Storage::loadSettings(settings);
    applySettings();
    screen.begin();
    lastIterTime = millis();
}

void loop() {
    uint32_t t = millis();
    M5.update();
    if (inMenu) {
        menuButtons();
        if (inMenu) {
            menu.update();
        }
        return;
    }
    // centre: start / pause / resume; left: settings (idle) or quit (paused)
    if (M5.BtnB.wasPressed()) {
        switch (game.state()) {
        case Game::State::Idle:
            game.start();
            break;
        case Game::State::Playing:
            game.pause(t);
            break;
        case Game::State::Paused:
            game.resume(t);
            break;
        default:
            break;
        }
    }
    if (M5.BtnA.wasPressed() && game.state() == Game::State::Paused) {
        game.quit();
    }
    else if (M5.BtnA.wasPressed() && game.state() == Game::State::Idle) {
        Sound::sidetoneOff();
        menu.open(settings);
        inMenu = true;
        menu.update();
        return;
    }
    if (t - lastIterTime >= ITER_INTERVAL_MS) {
        iter(t);
        lastIterTime = t;
    }
    screen.update(game, settings);
}
