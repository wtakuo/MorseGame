#include "Words.h"

#include <Arduino.h>

#include "MorseCode.h"

namespace {

#define COUNT(a) (sizeof(a) / sizeof(a[0]))

const char *const words2[] = {
    "AN", "AT", "AS", "BE", "BY", "DO", "GO", "HE", "IF", "IN", "IS", "IT",
    "ME", "MY", "NO", "OF", "ON", "OR", "SO", "TO", "UP", "WE", "AI", "PC",
    "CW", "DX", "FM", "AM", "PM", "73", "88", "DR", "MR", "MS"
};
const char *const words3[] = {
    "AND", "ARE", "FOR", "THE", "YOU", "BUT", "NOT", "ALL", "ANY", "CAN",
    "HAD", "HER", "WAS", "ONE", "OUR", "OUT", "DAY", "GET", "HAS", "HIM",
    "CAT", "DOG", "TWO", "HAM", "ZAT", "TNX", "ODD", "QSO", "QRS", "QLF",
    "SKY", "POT", "JAM", "BEE", "CUP", "BOX", "KEY", "MAP", "RIG", "TOP",
    "RAT", "BUG", "WEB", "LOG", "LED", "BAT", "BAG", "JAR", "VET", "ROW"
};
const char *const words4[] = {
    "THAT", "WITH", "HAVE", "THIS", "FROM", "THEY", "KNOW", "WERE", "YOUR",
    "WHAT", "WHEN", "MAKE", "LIKE", "TIME", "JUST", "GOOD", "COME", "WANT",
    "LOOK", "MORE", "WILL", "HOME", "WORK", "CALL", "SOME", "THEM", "VERY",
    "OVER", "EVEN", "BACK", "SUCH", "FEEL", "LONG", "PART", "HELP", "WORD",
    "TURN", "CASE", "FACT", "AREA", "KIND", "HEAD", "BODY", "PAGE", "TEST",
    "FIND", "SIDE", "TYPE", "FORM", "VIEW", "DATA", "NAME", "NICE", "COLD",
    "GAME", "FINE", "HIGH", "HARD", "FAST", "SURE", "REAL", "EASY", "FULL"
};
const char *const words5[] = {
    "ABOUT", "THERE", "WHICH", "THEIR", "WOULD", "OTHER", "COULD", "FIRST",
    "THOSE", "MIGHT", "AFTER", "SHALL", "EVERY", "UNDER", "WORLD", "RIGHT",
    "PLACE", "SMALL", "FOUND", "GREAT", "HOUSE", "POINT", "THING", "WHERE",
    "WHILE", "BEING", "GROUP", "SEEMS", "BELOW", "LEVEL", "LEAVE",
    "STAND", "ORDER", "LIGHT", "WATER", "NIGHT", "STORY", "STILL", "YOUNG",
    "MONTH", "BLACK", "SHORT", "HEART", "TABLE", "CLASS", "VOICE",
    "POWER", "FIELD", "SPACE", "BOARD", "BREAK", "CHAIR", "CARRY", "CATCH",
    "COUNT", "CROSS", "DANCE", "DREAM", "DRINK", "EARLY", "ENTER", "FAITH",
    "FINAL", "FORCE", "FRONT", "GLASS", "GREEN", "HAPPY", "IDEAL", "JOKER",
    "LAUGH", "MARCH", "MUSIC", "OFFER", "PARTY", "PEACE", "PLAIN"
};

}  // namespace

const char *pickChallenge(int level, bool digits) {
    switch (level) {
    case 1:
        // morseTable starts with the letters, then the digits
        return morseTable[random(0, NUM_LETTERS + (digits ? NUM_DIGITS : 0))].text;
    case 2:
        return words2[random(0, COUNT(words2))];
    case 3:
        return words3[random(0, COUNT(words3))];
    case 4:
        return words4[random(0, COUNT(words4))];
    default:
        return words5[random(0, COUNT(words5))];
    }
}
