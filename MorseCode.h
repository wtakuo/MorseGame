// Morse code table. Plain C++ (no Arduino) so that it can be built on a PC
// by tools/decoder_host.cpp.
#pragma once

struct MorseChar {
    const char *text;
    const char *code;  // '.' = dot, '-' = dash
};

// The first NUM_LETTERS entries are A-Z, followed by NUM_DIGITS digits 0-9.
extern const MorseChar morseTable[];
extern const int MORSE_TABLE_SIZE;
const int NUM_LETTERS = 26;
const int NUM_DIGITS = 10;

// Text of the character with the given code, or nullptr if there is none.
const char *morseLookup(const char *code);
