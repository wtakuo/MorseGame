// Game state: challenges, judgement, score, lives and levels.
#pragma once

#include <Arduino.h>

// Parameters that differ between difficulties (see rulesFor() in Settings).
// The defaults are the intermediate rules.
struct GameRules {
    static const int MAX_LEVEL = 5;
    uint32_t timeoutMs[MAX_LEVEL] = {4000, 7000, 10000, 12000, 15000};
    int successesPerLevel = 10;   // correct answers to go up a level
    int lives = 5;
    bool digitsInLevel1 = true;   // level 1 asks digits as well as letters
};

class Game {
public:
    enum class State { Idle, Playing, GameOver };

    // What changed since the last takeChanges(), for the screen.
    enum Change : uint8_t {
        SCORE = 1,     // score or lives
        MESSAGE = 2,   // state or challenge
        TIME_BAR = 4,  // remaining time
    };

    explicit Game(const GameRules &rules) : rules_(rules), lives_(rules.lives) {}

    // Rules for the next game (takes effect at start()).
    void setRules(const GameRules &rules);

    // Start a new game from the idle screen.
    void start();

    // Input from the decoder.
    void addCharacter(const char *text) { inputWord_ += text; }
    void endWord() { wordCompleted_ = true; }

    // Advance the game; call every iteration.
    void update(uint32_t now);

    uint8_t takeChanges();

    // True once after a game has ended, with score() as the final score.
    bool takeEnded();

    State state() const { return state_; }
    int score() const { return score_; }
    int lives() const { return lives_; }
    int level() const { return level_; }
    const String &challenge() const { return challenge_; }
    uint32_t timeout() const { return timeout_; }
    long remainingTime() const { return remainingTime_; }

private:
    static const uint32_t TIME_BAR_INTERVAL_MS = 200;
    static const uint32_t GAME_OVER_MS = 5000;   // GAME OVER is shown this long

    void newChallenge(uint32_t now);

    GameRules rules_;
    bool ended_ = false;
    State state_ = State::Idle;
    uint8_t changes_ = SCORE | MESSAGE;
    int score_ = 0;
    int lives_ = 0;
    int level_ = 1;
    int successCount_ = 0;
    String challenge_;
    String inputWord_;
    bool wordCompleted_ = false;
    uint32_t timeout_ = 0;
    uint32_t startTime_ = 0;
    long remainingTime_ = 0;
    uint32_t lastTimeBarUpdate_ = 0;
    uint32_t gameOverTime_ = 0;
};
