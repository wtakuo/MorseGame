// Challenge words for each level.
#pragma once

// A random challenge for the level (1: a letter, or also a digit if
// `digits`; 2-5: a word of that many characters).
const char *pickChallenge(int level, bool digits);
