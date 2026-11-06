#include "Game.h"

#include "Sound.h"
#include "Words.h"

void Game::start() {
    if (state_ != State::Idle) {
        return;
    }
    level_ = 1;
    timeout_ = 0;
    lives_ = rules_.lives;
    score_ = 0;
    challenge_ = "";
    inputWord_ = "";
    wordCompleted_ = false;
    successCount_ = 0;
    changes_ |= SCORE;
    state_ = State::Playing;
}

void Game::pause(uint32_t now) {
    if (state_ != State::Playing) {
        return;
    }
    state_ = State::Paused;
    pauseTime_ = now;
    changes_ |= MESSAGE;
}

void Game::resume(uint32_t now) {
    if (state_ != State::Paused) {
        return;
    }
    // move the start of the challenge on by the time spent paused
    uint32_t paused = now - pauseTime_;
    startTime_ += paused;
    lastTimeBarUpdate_ += paused;
    // forget what was keyed while paused
    inputWord_ = "";
    wordCompleted_ = false;
    state_ = State::Playing;
    changes_ |= MESSAGE | TIME_BAR;
}

void Game::quit() {
    if (state_ != State::Paused) {
        return;
    }
    ended_ = true;
    state_ = State::Idle;
    changes_ |= SCORE | MESSAGE;
}

void Game::setRules(const GameRules &rules) {
    rules_ = rules;
    if (state_ == State::Idle) {
        lives_ = rules_.lives;
        changes_ |= SCORE;
    }
}

bool Game::takeEnded() {
    bool e = ended_;
    ended_ = false;
    return e;
}

uint8_t Game::takeChanges() {
    uint8_t c = changes_;
    changes_ = 0;
    return c;
}

void Game::newChallenge(uint32_t now) {
    challenge_ = pickChallenge(level_, rules_.digitsInLevel1);
    timeout_ = rules_.timeoutMs[level_ - 1];
    startTime_ = now;
    changes_ |= MESSAGE;
    // draw the full bar right away
    lastTimeBarUpdate_ = now - TIME_BAR_INTERVAL_MS;
}

void Game::update(uint32_t now) {
    if (state_ == State::Playing) {
        // a word gap after the input was already judged or cleared ends an
        // empty word; ignore it
        if (wordCompleted_ && inputWord_.equals("")) {
            wordCompleted_ = false;
        }
        if (challenge_.equals("")) {
            newChallenge(now);
        }
        else if ((wordCompleted_ || inputWord_.equals(challenge_)) &&
                 now < startTime_ + timeout_) {
            // judge a correct word as soon as its last character is decoded;
            // a wrong one still waits for the word gap
            if (challenge_.equals(inputWord_)) {
                Sound::correct();
                score_ += level_ * 10;
                successCount_++;
                if (level_ < GameRules::MAX_LEVEL &&
                    successCount_ >= rules_.successesPerLevel) {
                    level_++;
                    successCount_ = 0;
                }
                challenge_ = "";
                changes_ |= SCORE | MESSAGE;
            }
            else {
                Sound::wrong();
            }
            inputWord_ = "";
            wordCompleted_ = false;
        }
        else if (now >= startTime_ + timeout_) {
            lives_--;
            Sound::timeUp();
            if (lives_ <= 0) {
                state_ = State::GameOver;
                ended_ = true;
                gameOverTime_ = now + GAME_OVER_MS;
                changes_ |= MESSAGE;
            }
            else {
                challenge_ = "";
            }
            inputWord_ = "";
            wordCompleted_ = false;
            changes_ |= SCORE;
        }
        if (now - lastTimeBarUpdate_ >= TIME_BAR_INTERVAL_MS) {
            remainingTime_ = max(0L, (long)timeout_ - (long)(now - startTime_));
            changes_ |= TIME_BAR;
            lastTimeBarUpdate_ = now;
        }
    }
    else if (state_ == State::GameOver) {
        if (now >= gameOverTime_) {
            state_ = State::Idle;
            changes_ |= MESSAGE;
        }
    }
}
