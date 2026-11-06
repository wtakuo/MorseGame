#include "Screen.h"

#include <M5Unified.h>

void Screen::begin() {
    w_ = M5.Display.width();
    h_ = M5.Display.height();

    keylampX_ = KEYLAMP_SIZE + 1;
    keylampY_ = h_ - KEYLAMP_SIZE - 2;
    M5.Display.setFont(&fonts::Font0);
    M5.Display.setTextSize(INPUT_TEXT_SCALE);
    inputTextX_ = KEYLAMP_SIZE * 2 + 7;
    inputTextY_ = h_ - M5.Display.fontHeight(M5.Display.getFont());
    maxInputText_ = (w_ - inputTextX_) / M5.Display.textWidth("W");

    M5.Display.setTextSize(2);
    scoreX_ = w_ / 2 + 16;
    scoreH_ = SCORE_LINE_SPACING + M5.Display.fontHeight(M5.Display.getFont());

    messageY_ = h_ / 2 - 50;
    messageH_ = 96;
    timeBarY_ = messageY_ + messageH_ + 2;
    redrawAll();
}

void Screen::redrawAll() {
    redrawAll_ = true;
}

void Screen::drawTitle(const Settings &settings) {
    M5.Display.setFont(&fonts::Font2);
    M5.Display.setTextColor(CYAN, BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(0, 0);
    M5.Display.print("Morse Game");
    // the current settings, under the title
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(LIGHTGREY, BLACK);
    M5.Display.setCursor(0, scoreH_ + 2);
    M5.Display.printf("%s  %d WPM  %s", keyModeName(settings.keyMode), settings.wpm,
                      difficultyName(settings.difficulty));
}

void Screen::setKeyLamp(bool on) {
    keyOn_ = on;
    keyLampChanged_ = true;
}

void Screen::appendInput(const char *text) {
    inputText_ += text;
    if (inputText_.length() > (unsigned)maxInputText_) {
        inputText_.remove(0, inputText_.length() - maxInputText_);
    }
    inputTextChanged_ = true;
}

void Screen::update(Game &game, const Settings &settings) {
    uint8_t changes = game.takeChanges();
    if (redrawAll_) {
        changes = Game::SCORE | Game::MESSAGE | Game::TIME_BAR;
        keyLampChanged_ = inputTextChanged_ = true;
    }
    if (!changes && !keyLampChanged_ && !inputTextChanged_) {
        return;
    }
    M5.Display.startWrite();
    if (redrawAll_) {
        M5.Display.fillScreen(BLACK);
        drawTitle(settings);
        redrawAll_ = false;
    }
    M5.Display.fillCircle(keylampX_, keylampY_, KEYLAMP_SIZE, keyOn_ ? GREEN : BLACK);
    keyLampChanged_ = false;
    if (inputTextChanged_) {
        M5.Display.setFont(&fonts::Font0);
        M5.Display.setTextSize(INPUT_TEXT_SCALE);
        M5.Display.setCursor(inputTextX_, inputTextY_);
        M5.Display.fillRect(inputTextX_, inputTextY_, w_ - inputTextX_,
                            M5.Display.fontHeight(M5.Display.getFont()), BLACK);
        M5.Display.print(inputText_);
        inputTextChanged_ = false;
    }
    if (changes & Game::SCORE) {
        drawScore(game);
    }
    if (changes & Game::MESSAGE) {
        drawMessage(game, settings);
    }
    if ((changes & Game::TIME_BAR) && game.state() == Game::State::Playing) {
        drawTimeBar(game);
    }
    M5.Display.endWrite();
}

void Screen::drawScore(const Game &game) {
    M5.Display.fillRect(scoreX_, 0, w_ - scoreX_, scoreH_, BLACK);
    M5.Display.setTextColor(WHITE, BLACK);
    M5.Display.setFont(&fonts::Font0);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(scoreX_, 0);
    M5.Display.printf("Score %04d", game.score());
    M5.Display.setCursor(scoreX_, SCORE_LINE_SPACING);
    M5.Display.printf("Lives %d", game.lives());
}

void Screen::drawMessage(const Game &game, const Settings &settings) {
    if (game.state() == Game::State::Idle) {
        M5.Display.fillRect(0, messageY_, w_, messageH_, BLACK);
        M5.Display.setTextColor(GREEN, BLACK);
        M5.Display.setFont(&fonts::Font2);
        M5.Display.setTextSize(1);
        char highScore[32];
        snprintf(highScore, sizeof(highScore), "HIGH SCORE  %04d",
                 Storage::highScore(settings.difficulty));
        const char *lines[] = {"CENTER BUTTON: START", "LEFT BUTTON: SETTINGS", highScore};
        int hh = M5.Display.fontHeight(M5.Display.getFont());
        int y = messageY_ + (messageH_ - 3 * (hh + 6)) / 2;
        for (const char *line : lines) {
            M5.Display.setCursor((w_ - M5.Display.textWidth(line)) / 2, y);
            M5.Display.print(line);
            y += hh + 6;
        }
    }
    else if (game.state() == Game::State::GameOver) {
        M5.Display.fillRect(0, messageY_, w_, messageH_, RED);
        M5.Display.setTextColor(WHITE, RED);
        M5.Display.setFont(&fonts::Font4);
        const char *gameOverText = "GAME OVER";
        int ww = M5.Display.textWidth(gameOverText);
        int hh = M5.Display.fontHeight(M5.Display.getFont());
        M5.Display.setCursor((w_ - ww) / 2, (h_ - hh) / 2);
        M5.Display.print(gameOverText);
    }
    else {
        M5.Display.fillRect(0, messageY_, w_, messageH_, BLACK);
        M5.Display.setTextColor(YELLOW, BLACK);
        M5.Display.setFont(&fonts::Font4);
        int ww = M5.Display.textWidth(game.challenge());
        int hh = M5.Display.fontHeight(M5.Display.getFont());
        M5.Display.setCursor((w_ - ww) / 2, (h_ - hh) / 2);
        M5.Display.print(game.challenge());
    }
    if (game.state() != Game::State::Playing) {
        M5.Display.drawFastHLine(0, timeBarY_, w_, BLACK);
    }
}

void Screen::drawTimeBar(const Game &game) {
    int dx = w_ * (game.remainingTime() / 2.0) / game.timeout();
    M5.Display.drawFastHLine(0, timeBarY_, w_, BLACK);
    M5.Display.drawFastHLine(w_ / 2 - dx, timeBarY_, 2 * dx, YELLOW);
}
