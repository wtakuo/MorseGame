#include "MorseDecoder.h"

#include <math.h>

#include <algorithm>

#include "MorseCode.h"

// ---------------------------------------------------------------- StraightKey

bool StraightKey::update(uint32_t now, bool rawDown) {
    if (rawDown != rawDown_) {
        rawDown_ = rawDown;
        rawChange_ = now;
    }
    if (rawDown != down_ && now - rawChange_ >= DEBOUNCE_MS) {
        if (!rawDown) {
            markLength_ = rawChange_ - lastChange_;
        }
        down_ = rawDown;
        lastChange_ = rawChange_;
        return true;
    }
    return false;
}

// --------------------------------------------------------------- SpeedTracker

void SpeedTracker::reset(float dotMs) {
    avgDot_ = dotMs;
    avgDash_ = 3 * dotMs;
    historyCount_ = 0;
    historyNext_ = 0;
    updateThresholds();
}

void SpeedTracker::updateThresholds() {
    // keep the averages within the allowed speed and dash/dot ratio; when
    // only one kind of mark has been seen, the other one follows it
    avgDash_ = std::min(std::max(avgDash_, MIN_DASH_RATIO * avgDot_), MAX_DASH_RATIO * avgDot_);
    avgDot_ = std::min(std::max(avgDot_, avgDash_ / MAX_DASH_RATIO), avgDash_ / MIN_DASH_RATIO);
    avgDot_ = std::min(std::max(avgDot_, (float)MIN_DOT_MS), (float)MAX_DOT_MS);
    avgDash_ = std::min(std::max(avgDash_, MIN_DASH_RATIO * avgDot_), MAX_DASH_RATIO * avgDot_);

    maxDot_ = (avgDot_ + avgDash_) / 2;
    charGap_ = 2.0 * avgDot_;
    wordGap_ = 6.0 * avgDot_;
}

bool SpeedTracker::classify(uint32_t length) {
    // resting on the key says nothing about the speed
    if (length > std::max(3 * avgDash_, (float)LONG_PRESS_MS)) {
        return true;
    }
    history_[historyNext_] = length;
    historyNext_ = (historyNext_ + 1) % HISTORY;
    if (historyCount_ < HISTORY) {
        historyCount_++;
    }

    uint32_t lo = history_[0], hi = history_[0];
    for (int i = 1; i < historyCount_; i++) {
        lo = std::min(lo, history_[i]);
        hi = std::max(hi, history_[i]);
    }
    if (hi >= BIMODAL_RATIO * lo) {
        // both dots and dashes in the history: split it into two clusters
        float th = sqrtf((float)lo * hi);
        float shortMean = lo, longMean = hi;
        for (int iter = 0; iter < 4; iter++) {
            float shortSum = 0, longSum = 0;
            int shortN = 0, longN = 0;
            for (int i = 0; i < historyCount_; i++) {
                if (history_[i] < th) {
                    shortSum += history_[i];
                    shortN++;
                }
                else {
                    longSum += history_[i];
                    longN++;
                }
            }
            shortMean = shortSum / shortN;  // lo < th <= hi, so neither is 0
            longMean = longSum / longN;
            th = (shortMean + longMean) / 2;
        }
        avgDot_ = shortMean;
        avgDash_ = longMean;
        updateThresholds();
        return length >= (uint32_t)maxDot_;
    }

    bool isDash = length >= (uint32_t)maxDot_;
    if (isDash) {
        avgDash_ += AVG_WEIGHT * (length - avgDash_);
    }
    else {
        avgDot_ += AVG_WEIGHT * (length - avgDot_);
    }
    updateThresholds();
    return isDash;
}

// ----------------------------------------------------------- CharacterDecoder

void CharacterDecoder::addMark(bool dash) {
    if (count_ < MAX_MARKS) {
        marks_[count_++] = dash ? '-' : '.';
        marks_[count_] = '\0';
    }
}

const char *CharacterDecoder::checkCharacterEnd(uint32_t gap, int charGap) {
    if (count_ == 0 || gap < (uint32_t)charGap) {
        return nullptr;
    }
    const char *text = morseLookup(marks_);
    marks_[0] = '\0';
    count_ = 0;
    inWord_ = true;
    return text ? text : "*";
}

bool CharacterDecoder::checkWordEnd(uint32_t gap, int wordGap) {
    if (!inWord_ || gap < (uint32_t)wordGap) {
        return false;
    }
    inWord_ = false;
    return true;
}
