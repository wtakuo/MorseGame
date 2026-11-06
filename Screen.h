// Drawing: title, score and lives, message area, time bar, key lamp and
// the line of decoded input.
#pragma once

#include <Arduino.h>

#include "Game.h"
#include "Settings.h"

class Screen {
public:
    // Compute the layout from the display size and draw the screen.
    void begin();
    // Clear the screen and draw everything again (after the settings menu).
    void redrawAll();

    void setKeyLamp(bool on);
    // Speed shown under the title: the tracked speed for a straight key,
    // the set speed for the keyer.
    void setWpm(int wpm);
    // Append decoded text (a character or " " at the end of a word).
    void appendInput(const char *text);

    // Redraw what changed in the game or the input; call every loop.
    void update(Game &game, const Settings &settings);

private:
    void drawTitle();
    void drawStatus(const Settings &settings);
    void drawScore(const Game &game);
    void drawMessage(const Game &game, const Settings &settings);
    void drawTimeBar(const Game &game);

    // Layout, set by begin() from the display size. The fonts have a fixed
    // pixel size, so the areas are placed relative to the screen edges and
    // centre rather than scaled.
    static const int KEYLAMP_SIZE = 10;
    static const int SCORE_LINE_SPACING = 22;  // from the score line to the lives line
    static constexpr float INPUT_TEXT_SCALE = 2.5;
    int w_ = 0, h_ = 0;
    int keylampX_ = 0, keylampY_ = 0;
    int inputTextX_ = 0, inputTextY_ = 0;
    int maxInputText_ = 0;          // characters that fit on the input line
    int scoreX_ = 0, scoreH_ = 0;   // score and lives, top right
    int messageY_ = 0, messageH_ = 0;  // challenge / idle / game over, around the centre
    int timeBarY_ = 0;              // remaining time, just below the message area

    String inputText_;
    bool keyOn_ = false;
    bool redrawAll_ = true;
    int wpm_ = 0;
    bool statusChanged_ = true;
    bool keyLampChanged_ = true;
    bool inputTextChanged_ = true;
};
