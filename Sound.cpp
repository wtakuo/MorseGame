#include "Sound.h"

#include <M5Unified.h>

namespace {

const float SIDETONE_FREQ = 600;

// Speaker virtual channels: the sidetone and the sound effects are kept
// apart so that releasing the key does not cut off an effect.
const int SIDETONE_CH = 0;
const int EFFECT_CH = 1;

// Play two notes in a row on the effect channel; the second one is queued
// behind the first (stop_current_sound = false), so this does not block.
void twoNotes(float freq1, uint32_t ms1, float freq2, uint32_t ms2) {
    M5.Speaker.tone(freq1, ms1, EFFECT_CH);
    M5.Speaker.tone(freq2, ms2, EFFECT_CH, false);
}

}  // namespace

namespace Sound {

void begin(uint8_t volume) {
    setVolume(volume);
}

void setVolume(uint8_t volume) {
    M5.Speaker.setVolume(volume);
}

void sidetoneOn() {
    M5.Speaker.tone(SIDETONE_FREQ, UINT32_MAX, SIDETONE_CH);
}

void sidetoneOff() {
    M5.Speaker.stop(SIDETONE_CH);
}

void correct() {
    M5.Speaker.tone(1600, 100, EFFECT_CH);
}

void wrong() {
    twoNotes(1000, 50, 800, 50);
}

void click() {
    M5.Speaker.tone(2000, 15, EFFECT_CH);
}

void timeUp() {
    twoNotes(800, 500, 700, 500);
}

}  // namespace Sound
