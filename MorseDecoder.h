// Straight key decoding: debounce, speed tracking and character assembly.
//
// Plain C++ with times passed in by the caller (no Arduino), so that the
// same code runs in tools/decoder_host.cpp for the keying simulator. Keep
// tools/morse_sim.py (WindowMeans) in step with SpeedTracker.
#pragma once

#include <stdint.h>

// Debounces a straight key. A change is accepted once the pin has been
// stable for DEBOUNCE_MS; its time is when the pin settled, so the delay
// cancels out in the mark and gap lengths.
class StraightKey {
public:
    static const uint32_t DEBOUNCE_MS = 10;

    // Feed the raw pin state (true = pressed). Returns true when the
    // debounced state changes.
    bool update(uint32_t now, bool rawDown);

    bool isDown() const { return down_; }
    uint32_t lastChangeTime() const { return lastChange_; }
    // Length of the last mark, valid after a release.
    uint32_t lastMarkLength() const { return markLength_; }

private:
    bool down_ = false;
    bool rawDown_ = false;
    uint32_t rawChange_ = 0;
    uint32_t lastChange_ = 0;
    uint32_t markLength_ = 0;
};

// Follows the player's speed and tells dots from dashes.
//
// The dot and dash lengths are averaged separately and the dot/dash
// threshold sits midway between them, so a player whose dashes are shorter
// than 3 dots, or who holds the last dot of a letter a little longer, is
// still decoded correctly. When the last HISTORY marks contain both short
// and long ones, both averages are taken from a two-cluster split of them,
// so the estimate follows a new player within a character or two.
//
// The character and word gaps are measured in units estimated from the
// dashes (1/3 of a dash) and from the gaps between the marks of a
// character, not from the dots: some players flick very short dots (a
// quarter of a unit or less), which made the character gap too short and
// split their letters (J read as ETTT).
class SpeedTracker {
public:
    static const int MIN_DOT_MS = 25;        // about 48 WPM
    static const int MAX_DOT_MS = 300;       // about 4 WPM
    static constexpr float MIN_DASH_RATIO = 2.0f;
    static constexpr float MAX_DASH_RATIO = 8.0f;
    static constexpr float BIMODAL_RATIO = 2.0f;  // longest/shortest mark meaning "both kinds"
    static constexpr float AVG_WEIGHT = 0.25f;    // weight of a new mark in a running average
    static const uint32_t LONG_PRESS_MS = 1000;   // longer marks (and 3 dashes) are ignored
    static const int HISTORY = 8;

    explicit SpeedTracker(float dotMs) { reset(dotMs); }

    // Forget the history and start again from the given dot length.
    void reset(float dotMs);

    // Classify a mark as a dot (false) or a dash (true) and update the
    // estimate with it.
    bool classify(uint32_t length);
    // Tell the gap before a mark; gaps within a character update the unit.
    void gap(uint32_t length);

    float dotLength() const { return avgDot_; }
    float dashLength() const { return avgDash_; }
    float gapLength() const { return avgGap_; }
    // The unit of the character and word gaps (for the WPM shown).
    float unitLength() const { return unit_; }
    int maxDot() const { return maxDot_; }
    int charGap() const { return charGap_; }
    int wordGap() const { return wordGap_; }

private:
    void updateThresholds();

    float avgDot_ = 0;
    float avgDash_ = 0;
    float avgGap_ = 0;     // gaps between the marks of a character
    float unit_ = 0;
    uint32_t history_[HISTORY] = {};
    int historyCount_ = 0;
    int historyNext_ = 0;
    int maxDot_ = 0;
    int charGap_ = 0;
    int wordGap_ = 0;
};

// Iambic keyer for a paddle. Generates dits and dahs at a fixed speed
// while a paddle is held; squeezing both paddles alternates them.
//
// Opposite-paddle presses during an element (and the space after it) are
// latched. In mode B the latch is used, so releasing a squeeze sends one
// more element of the other kind; in mode A only the paddles still held at
// the end of the space count, so a squeeze stops with the current element.
class IambicKeyer {
public:
    enum class Mode { A, B };

    void configure(Mode mode, uint32_t unitMs);

    // Feed the (debounced) paddle states. Returns true when the keying
    // output (sidetone) turns on or off.
    bool update(uint32_t now, bool dit, bool dah);

    bool isDown() const { return state_ == State::Mark; }
    // When the output last turned on or off (the exact element boundary,
    // not the sampling time).
    uint32_t lastChangeTime() const { return lastChange_; }
    // Kind of the element that just ended (valid after the output turns off).
    bool lastElementDash() const { return dash_; }

    int charGap() const { return 2 * unit_; }
    int wordGap() const { return 6 * unit_; }

private:
    enum class State { Idle, Mark, Space };

    void startElement(uint32_t t, bool dash);

    Mode mode_ = Mode::A;
    uint32_t unit_ = 80;
    State state_ = State::Idle;
    bool dash_ = false;
    bool ditMemory_ = false;
    bool dahMemory_ = false;
    uint32_t until_ = 0;       // end of the current mark or space
    uint32_t lastChange_ = 0;
};

// Collects dots and dashes into characters and characters into words.
class CharacterDecoder {
public:
    // Marks of a character beyond MAX_MARKS are dropped; such a character is
    // decoded as unknown because no code in morseTable is that long.
    static const int MAX_MARKS = 9;

    void addMark(bool dash);

    // Call while the key is up, with the time since it was released. When
    // the character gap has passed, returns the decoded text ("*" for an
    // unknown code); otherwise nullptr.
    const char *checkCharacterEnd(uint32_t gap, int charGap);

    // Call while the key is up. Returns true once when the word gap has
    // passed after at least one character.
    bool checkWordEnd(uint32_t gap, int wordGap);

private:
    char marks_[MAX_MARKS + 1] = "";
    int count_ = 0;
    bool inWord_ = false;
};
