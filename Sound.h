// Sidetone and sound effects.
#pragma once

#include <stdint.h>

namespace Sound {

void begin(uint8_t volume);
void setVolume(uint8_t volume);

void sidetoneOn();
void sidetoneOff();

void correct();   // the word was keyed correctly
void wrong();     // the word was keyed wrongly
void timeUp();    // the time ran out
void click();     // a button on the settings screen

}  // namespace Sound
