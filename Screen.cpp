#include "Screen.h"

#include <M5Unified.h>

#include "ButtonBar.h"
#include "Version.h"

void Screen::splash() {
    const uint32_t SPLASH_MS = 3000;
    int w = M5.Display.width(), h = M5.Display.height();
    M5.Display.fillScreen(BLACK);
    auto centred = [w](const char *text, int y) {
        M5.Display.setCursor((w - M5.Display.textWidth(text)) / 2, y);
        M5.Display.print(text);
    };
    M5.Display.setFont(&fonts::Font4);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(CYAN, BLACK);
    int titleH = M5.Display.fontHeight(M5.Display.getFont());
    int y = h / 2 - titleH;
    centred("Morse Game", y);
    y += titleH + 8;
    M5.Display.setFont(&fonts::Font2);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(WHITE, BLACK);
    centred("Version " MORSE_GAME_VERSION, y);
    y += M5.Display.fontHeight(M5.Display.getFont()) + 16;
    M5.Display.setTextColor(LIGHTGREY, BLACK);
    centred(MORSE_GAME_AUTHOR, y);
    delay(SPLASH_MS);
}

void Screen::begin() {
    w_ = M5.Display.width();
    h_ = M5.Display.height();

    // the input line sits just above the button labels
    M5.Display.setFont(&fonts::Font0);
    M5.Display.setTextSize(INPUT_TEXT_SCALE);
    int inputH = M5.Display.fontHeight(M5.Display.getFont());
    inputTextX_ = KEYLAMP_SIZE * 2 + 7;
    inputTextY_ = h_ - ButtonBar::HEIGHT - inputH - 2;
    keylampX_ = KEYLAMP_SIZE + 1;
    keylampY_ = inputTextY_ + inputH / 2 - 1;
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

void Screen::setWpm(int wpm) {
    if (wpm != wpm_) {
        wpm_ = wpm;
        statusChanged_ = true;
    }
}

void Screen::drawTitle() {
    M5.Display.setFont(&fonts::Font2);
    M5.Display.setTextColor(CYAN, BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(0, 0);
    M5.Display.print("Morse Game");
}

// The current settings, under the title.
void Screen::drawStatus(const Settings &settings) {
    M5.Display.setFont(&fonts::Font2);
    M5.Display.setTextSize(1);
    int y = scoreH_ + 2;
    M5.Display.fillRect(0, y, w_, M5.Display.fontHeight(M5.Display.getFont()), BLACK);
    M5.Display.setTextColor(LIGHTGREY, BLACK);
    M5.Display.setCursor(0, y);
    M5.Display.printf("%s  %d WPM  LEVEL %s", keyModeName(settings.keyMode), wpm_,
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
        keyLampChanged_ = inputTextChanged_ = statusChanged_ = true;
    }
    if (!changes && !keyLampChanged_ && !inputTextChanged_ && !statusChanged_) {
        return;
    }
    M5.Display.startWrite();
    if (redrawAll_) {
        M5.Display.fillScreen(BLACK);
        drawTitle();
        shownButtons_ = -1;
        redrawAll_ = false;
    }
    if (statusChanged_) {
        drawStatus(settings);
        statusChanged_ = false;
    }
    M5.Display.fillCircle(keylampX_, keylampY_, KEYLAMP_SIZE, keyOn_ ? GREEN : BLACK);
    keyLampChanged_ = false;
    if (inputTextChanged_) {
        // light blue, to set the decoded text apart from the white labels
        const uint16_t INPUT_TEXT_COLOR = M5.Display.color565(100, 180, 255);
        M5.Display.setFont(&fonts::Font0);
        M5.Display.setTextSize(INPUT_TEXT_SCALE);
        M5.Display.setTextColor(INPUT_TEXT_COLOR, BLACK);
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
        drawButtons(game);
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

// Text sizes are set explicitly: M5GFX keeps the last size across fonts,
// and the old code drew these at size 2, left over from the score.
void Screen::drawMessage(const Game &game, const Settings &settings) {
    auto centred = [this](const char *text, int y) {
        M5.Display.setCursor((w_ - M5.Display.textWidth(text)) / 2, y);
        M5.Display.print(text);
    };
    if (game.state() == Game::State::Idle) {
        M5.Display.fillRect(0, messageY_, w_, messageH_, BLACK);
        M5.Display.setTextColor(GREEN, BLACK);
        M5.Display.setFont(&fonts::Font2);
        M5.Display.setTextSize(2);
        int hh = M5.Display.fontHeight(M5.Display.getFont());
        int y = messageY_ + 16;
        centred("PRESS START", y);
        char highScore[32];
        snprintf(highScore, sizeof(highScore), "HIGH SCORE  %04d",
                 Storage::highScore(settings.difficulty));
        M5.Display.setTextColor(WHITE, BLACK);
        centred(highScore, y + hh + 8);
    }
    else if (game.state() == Game::State::Paused) {
        M5.Display.fillRect(0, messageY_, w_, messageH_, DARKGREY);
        M5.Display.setTextColor(WHITE, DARKGREY);
        M5.Display.setFont(&fonts::Font4);
        M5.Display.setTextSize(2);
        int hh = M5.Display.fontHeight(M5.Display.getFont());
        centred("PAUSED", messageY_ + (messageH_ - hh) / 2);
    }
    else if (game.state() == Game::State::GameOver) {
        M5.Display.fillRect(0, messageY_, w_, messageH_, RED);
        M5.Display.setTextColor(WHITE, RED);
        M5.Display.setFont(&fonts::Font4);
        M5.Display.setTextSize(2);
        int hh = M5.Display.fontHeight(M5.Display.getFont());
        centred("GAME OVER", (h_ - hh) / 2);
    }
    else {
        M5.Display.fillRect(0, messageY_, w_, messageH_, BLACK);
        M5.Display.setTextColor(YELLOW, BLACK);
        M5.Display.setFont(&fonts::Font4);
        M5.Display.setTextSize(2);
        int hh = M5.Display.fontHeight(M5.Display.getFont());
        centred(game.challenge().c_str(), (h_ - hh) / 2);
    }
    if (game.state() != Game::State::Playing) {
        M5.Display.drawFastHLine(0, timeBarY_, w_, BLACK);
    }
}

// What the buttons do in each state.
void Screen::drawButtons(const Game &game) {
    int state = (int)game.state();
    if (state == shownButtons_) {
        return;
    }
    shownButtons_ = state;
    switch (game.state()) {
    case Game::State::Idle:
        ButtonBar::draw("SETTINGS", "START", nullptr);
        break;
    case Game::State::Playing:
        ButtonBar::draw(nullptr, "PAUSE", nullptr);
        break;
    case Game::State::Paused:
        ButtonBar::draw("QUIT", "RESUME", nullptr);
        break;
    default:
        ButtonBar::draw(nullptr, nullptr, nullptr);
        break;
    }
}

void Screen::drawTimeBar(const Game &game) {
    int dx = w_ * (game.remainingTime() / 2.0) / game.timeout();
    M5.Display.drawFastHLine(0, timeBarY_, w_, BLACK);
    M5.Display.drawFastHLine(w_ / 2 - dx, timeBarY_, 2 * dx, YELLOW);
}
