// Morse Game for M5Stack

#include <M5Unified.h>

#if !defined(KEY_PIN)
#if defined(ARDUINO_M5STACK_Core2)
// M5Stack Core2 Grove Port A
#define KEY_PIN 33
#elif defined(ARDUINO_M5STACK_FIRE)
// M5Stack Fire Grove Port B
#define KEY_PIN 36
#else
// M5Stack CoreS3 Grove Port B
#define KEY_PIN 8
#endif
#endif

String morsePlain[] = {
    "A",    "B",    "C",    "D",    "E",    "F",    "G",    "H",
    "I",    "J",    "K",    "L",    "M",    "N",    "O",    "P",
    "Q",    "R",    "S",    "T",    "U",    "V",    "W",    "X",
    "Y",    "Z",    "0",    "1",    "2",    "3",    "4",    "5",
    "6",    "7",    "8",    "9",    "!",    ".",    "?",    "/",    
    "[AR]", "[BK]", "[BT]", "[HH]", "[SK]"
};

byte morseCode[][9] = {
    //  1 = dot, 2 = dash, 0 = not used
    {1, 2, 0, 0, 0, 0, 0, 0, 0},  //  [0]    A   .-
    {2, 1, 1, 1, 0, 0, 0, 0, 0},  //  [1]    B   -...
    {2, 1, 2, 1, 0, 0, 0, 0, 0},  //  [2]    C   -.-.
    {2, 1, 1, 0, 0, 0, 0, 0, 0},  //  [3]    D   -..
    {1, 0, 0, 0, 0, 0, 0, 0, 0},  //  [4]    E   .
    {1, 1, 2, 1, 0, 0, 0, 0, 0},  //  [5]    F   ..-.
    {2, 2, 1, 0, 0, 0, 0, 0, 0},  //  [6]    G   --.
    {1, 1, 1, 1, 0, 0, 0, 0, 0},  //  [7]    H   ....
    {1, 1, 0, 0, 0, 0, 0, 0, 0},  //  [8]    I   ..
    {1, 2, 2, 2, 0, 0, 0, 0, 0},  //  [9]    J   .---
    {2, 1, 2, 0, 0, 0, 0, 0, 0},  //  [10]   K   -.-
    {1, 2, 1, 1, 0, 0, 0, 0, 0},  //  [11]   L   .-..
    {2, 2, 0, 0, 0, 0, 0, 0, 0},  //  [12]   M   --
    {2, 1, 0, 0, 0, 0, 0, 0, 0},  //  [13]   N   -.
    {2, 2, 2, 0, 0, 0, 0, 0, 0},  //  [14]   O   ---
    {1, 2, 2, 1, 0, 0, 0, 0, 0},  //  [15]   P   .--.
    {2, 2, 1, 2, 0, 0, 0, 0, 0},  //  [16]   Q   --.-
    {1, 2, 1, 0, 0, 0, 0, 0, 0},  //  [17]   R   .-.
    {1, 1, 1, 0, 0, 0, 0, 0, 0},  //  [18]   S   ...
    {2, 0, 0, 0, 0, 0, 0, 0, 0},  //  [19]   T   -
    {1, 1, 2, 0, 0, 0, 0, 0, 0},  //  [20]   U   ..-
    {1, 1, 1, 2, 0, 0, 0, 0, 0},  //  [21]   V   ...-
    {1, 2, 2, 0, 0, 0, 0, 0, 0},  //  [22]   W   .--
    {2, 1, 1, 2, 0, 0, 0, 0, 0},  //  [23]   X   -..-
    {2, 1, 2, 2, 0, 0, 0, 0, 0},  //  [24]   Y   -.--
    {2, 2, 1, 1, 0, 0, 0, 0, 0},  //  [25]   Z   --..
    {2, 2, 2, 2, 2, 0, 0, 0, 0},  //  [26]   0   -----
    {1, 2, 2, 2, 2, 0, 0, 0, 0},  //  [27]   1   .----
    {1, 1, 2, 2, 2, 0, 0, 0, 0},  //  [28]   2   ..---
    {1, 1, 1, 2, 2, 0, 0, 0, 0},  //  [29]   3   ...--
    {1, 1, 1, 1, 2, 0, 0, 0, 0},  //  [30]   4   ....-
    {1, 1, 1, 1, 1, 0, 0, 0, 0},  //  [31]   5   .....
    {2, 1, 1, 1, 1, 0, 0, 0, 0},  //  [32]   6   -....
    {2, 2, 1, 1, 1, 0, 0, 0, 0},  //  [33]   7   --...
    {2, 2, 2, 1, 1, 0, 0, 0, 0},  //  [34]   8   ---..
    {2, 2, 2, 2, 1, 0, 0, 0, 0},  //  [35]   9   ----.
    {2, 1, 2, 1, 2, 2, 0, 0, 0},  //  [36]   !   -.-.--
    {1, 2, 1, 2, 1, 2, 0, 0, 0},  //  [37]   .   .-.-.-
    {1, 1, 2, 2, 1, 1, 0, 0, 0},  //  [38]   ?   ..--..
    {2, 1, 1, 2, 1, 0, 0, 0, 0},  //  [39]   /   -..-.
    {1, 2, 1, 2, 1, 0, 0, 0, 0},  //  [40]   [AR] .-.-.
    {2, 1, 1, 1, 2, 1, 2, 0, 0},  //  [41]   [BK] -...-.-
    {2, 1, 1, 1, 2, 0, 0, 0, 0},  //  [42]   [BT] -...-
    {1, 1, 1, 1, 1, 1, 1, 1, 0},  //  [43]   [HH] ........
    {1, 1, 1, 2, 1, 2, 0, 0, 0},  //  [44]   [SK] ...-.-
};

String words2[] = {
    "AN", "AT", "AS", "BE", "BY", "DO", "GO", "HE", "IF", "IN", "IS", "IT",
    "ME", "MY", "NO", "OF", "ON", "OR", "SO", "TO", "UP", "WE", "AI", "PC",
    "CW", "DX", "FM", "AM", "PM", "73", "88", "DR", "MR", "MS"
};
String words3[] = {
    "AND", "ARE", "FOR", "THE", "YOU", "BUT", "NOT", "ALL", "ANY", "CAN",
    "HAD", "HER", "WAS", "ONE", "OUR", "OUT", "DAY", "GET", "HAS", "HIM",
    "CAT", "DOG", "TWO", "HAM", "ZAT", "TNX", "ODD", "QSO", "QRS", "QLF",
    "SKY", "POT", "JAM", "BEE", "CUP", "BOX", "KEY", "MAP", "RIG", "TOP",
    "RAT", "BUG", "WEB", "LOG", "LED", "BAT", "BAG", "JAR", "VET", "ROW"
};
String words4[] = {
    "THAT", "WITH", "HAVE", "THIS", "FROM", "THEY", "KNOW", "WERE", "YOUR",
    "WHAT", "WHEN", "MAKE", "LIKE", "TIME", "JUST", "GOOD", "COME", "WANT",
    "LOOK", "MORE", "WILL", "HOME", "WORK", "CALL", "SOME", "THEM", "VERY", "OVER", "EVEN", "BACK", "SUCH", "FEEL", "LONG", "PART", "HELP", "WORD", 
    "TURN", "CASE", "FACT", "AREA", "KIND", "HEAD", "BODY", "PAGE", "TEST", "FIND", "SIDE", "TYPE", "FORM", "VIEW", "DATA", "NAME", "NICE", "COLD",
    "GAME", "FINE", "HIGH", "HARD", "FAST", "SURE", "REAL", "EASY", "FULL"
};
String words5[] = {
    "ABOUT", "THERE", "WHICH", "THEIR", "WOULD", "OTHER", "COULD", "FIRST",
    "THOSE", "MIGHT", "AFTER", "SHALL", "EVERY", "UNDER", "WORLD", "RIGHT",
    "PLACE", "SMALL", "FOUND", "GREAT", "HOUSE", "POINT", "THING", "WHERE",
    "WHILE", "WORLD", "BEING", "GROUP", "SEEMS", "BELOW", "LEVEL", "LEAVE",
    "STAND", "ORDER", "LIGHT", "WATER", "NIGHT", "STORY", "STILL", "YOUNG",
    "MONTH", "BLACK", "SHORT", "LEVEL", "HEART", "TABLE", "CLASS", "VOICE",
    "POWER", "FIELD", "SPACE", "BOARD", "BREAK", "CHAIR", "CARRY", "CATCH",
    "COUNT", "CROSS", "DANCE", "DREAM", "DRINK", "EARLY", "ENTER", "FAITH",
    "FINAL", "FORCE", "FRONT", "GLASS", "GREEN", "HAPPY", "IDEAL", "JOKER", "LAUGH", "MARCH", "MUSIC", "OFFER", "ORDER", "PARTY", "PEACE", "PLAIN"
};

// Speed tracking. The dot and dash lengths are averaged separately and the
// dot/dash threshold sits midway between them, so a player whose dashes are
// shorter than 3 dots, or who holds the last dot of a letter a little longer,
// is still decoded correctly. When the last MARK_HISTORY marks contain both
// short and long ones, both averages are taken from a two-cluster split of
// them, so the estimate follows a new player within a character or two.
// Tuned with tools/morse_sim.py.
#define INITIAL_DOT_MS 80     // about 15 WPM
#define MIN_DOT_MS 25         // about 48 WPM
#define MAX_DOT_MS 300        // about 4 WPM
#define MIN_DASH_RATIO 2.0f
#define MAX_DASH_RATIO 4.0f
#define BIMODAL_RATIO 2.0f    // longest/shortest mark that means "both kinds"
#define AVG_WEIGHT 0.25f      // weight of a new mark in a running average
#define LONG_PRESS_MS 1000    // marks longer than this (and 3 dashes) are ignored
#define MARK_HISTORY 8

float avgDot = INITIAL_DOT_MS;
float avgDash = 3 * INITIAL_DOT_MS;
unsigned int markHistory[MARK_HISTORY];
byte markHistoryCount = 0;
byte markHistoryNext = 0;

int maxDotDuration;
int minDuration;
int minWordGap;
int minCharacterGap;

#define ITER_INTERVAL_MS 4
#define DRAW_INTERVAL_MS 50
#define TIME_BAR_INTERVAL_MS 200

unsigned long lastIterTime;
// unsigned long lastDrawTime;

int lastKeyState = HIGH;     // debounced key state
unsigned long lastChangeTime = 0;
int rawKeyState = HIGH;      // key state as last read from the pin
unsigned long rawChangeTime = 0;
bool stateNow = 0;
bool newDataFlag = 0;
bool lastMarkType = 0;
unsigned int lastMarkLength;

byte currentCharacter[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
byte bufferPosition = 0;
byte currentCharacterNumber;
bool spaceFlag = 0;

void setTiming() {
    // keep the averages within the allowed speed and dash/dot ratio; when
    // only one kind of mark has been seen, the other one follows it
    avgDash = constrain(avgDash, MIN_DASH_RATIO * avgDot, MAX_DASH_RATIO * avgDot);
    avgDot = constrain(avgDot, avgDash / MAX_DASH_RATIO, avgDash / MIN_DASH_RATIO);
    avgDot = constrain(avgDot, (float)MIN_DOT_MS, (float)MAX_DOT_MS);
    avgDash = constrain(avgDash, MIN_DASH_RATIO * avgDot, MAX_DASH_RATIO * avgDot);

    minDuration = 10;
    maxDotDuration = (avgDot + avgDash) / 2;
    minCharacterGap = 2.0 * avgDot;
    minWordGap = 6.0 * avgDot;
}

// Classify a mark as a dot (false) or a dash (true) and update the speed
// estimate with it.
bool classifyMark(unsigned int length) {
    // resting on the key says nothing about the speed
    if (length > max(3 * avgDash, (float)LONG_PRESS_MS)) {
        return true;
    }
    markHistory[markHistoryNext] = length;
    markHistoryNext = (markHistoryNext + 1) % MARK_HISTORY;
    if (markHistoryCount < MARK_HISTORY) {
        markHistoryCount++;
    }

    unsigned int lo = markHistory[0], hi = markHistory[0];
    for (byte i = 1; i < markHistoryCount; i++) {
        lo = min(lo, markHistory[i]);
        hi = max(hi, markHistory[i]);
    }
    if (hi >= BIMODAL_RATIO * lo) {
        // both dots and dashes in the history: split it into two clusters
        float th = sqrtf((float)lo * hi);
        float shortMean = lo, longMean = hi;
        for (byte iter = 0; iter < 4; iter++) {
            float shortSum = 0, longSum = 0;
            byte shortN = 0, longN = 0;
            for (byte i = 0; i < markHistoryCount; i++) {
                if (markHistory[i] < th) {
                    shortSum += markHistory[i];
                    shortN++;
                }
                else {
                    longSum += markHistory[i];
                    longN++;
                }
            }
            shortMean = shortSum / shortN;   // lo < th <= hi, so neither is 0
            longMean = longSum / longN;
            th = (shortMean + longMean) / 2;
        }
        avgDot = shortMean;
        avgDash = longMean;
        setTiming();
        return length >= maxDotDuration;
    }

    bool isDash = length >= maxDotDuration;
    if (isDash) {
        avgDash += AVG_WEIGHT * (length - avgDash);
    }
    else {
        avgDot += AVG_WEIGHT * (length - avgDot);
    }
    setTiming();
    return isDash;
}

#define TONE_FREQ 600

// Speaker virtual channels: the sidetone and the sound effects are kept
// apart so that releasing the key does not cut off an effect.
#define SIDETONE_CH 0
#define EFFECT_CH 1

int keylampSize = 10;

bool screenNeedsUpdate = true;
bool keyOn = false;

#define MAXINPUTTEXT 64

int w, h;
int keylampX, keylampY;
int inputTextX, inputTextY;
float inputTextScale = 2.5;
String inputText = "";
int maxInputText = 19;
bool inputTextChanged = true;
bool scoreChanged = true;
int score = 0;

enum GameState { GS_IDLE, GS_INIT, GS_PLAYING, GS_GAMEOVER };
enum GameState gameState = GS_IDLE;
int level = 1;
int timeout;
unsigned long startTime = 0;
long remainingTime = 0;
bool remainingTimeChanged = false;
unsigned long lastTimeBarUpdate = 0;
int lives = 5;
String challengeWord = "";
bool challengeWordChanged = false;
String inputWord = "";
bool wordCompleted = false;
int successCount = 0;
unsigned long gameoverTimer = 0;

void setDrawPosition() {
    keylampX = keylampSize + 1;
    keylampY = h - keylampSize - 2;
    M5.Display.setFont(&fonts::Font0);
    M5.Display.setTextSize(inputTextScale);
    inputTextX = keylampSize * 2 + 7;
    inputTextY = h - M5.Display.fontHeight(M5.Display.getFont());
}

void drawScreen() {
    M5.Display.startWrite();
    // M5.Display.fillScreen(BLACK);
    if (keyOn) {
        M5.Display.fillCircle(keylampX, keylampY, keylampSize, GREEN);
    }
    else {
        M5.Display.fillCircle(keylampX, keylampY, keylampSize, BLACK);
    }
    if (inputTextChanged) {
        M5.Display.setFont(&fonts::Font0);
        M5.Display.setTextSize(inputTextScale);
        // M5.Display.setTextColor(CYAN, BLACK);
        M5.Display.setCursor(inputTextX, inputTextY);
        M5.Display.fillRect(inputTextX, inputTextY,
                           w - inputTextX, M5.Display.fontHeight(M5.Display.getFont()),
                           BLACK);
        M5.Display.print(inputText);
        inputTextChanged = false;
    }
    if (scoreChanged) {
        M5.Display.fillRect(w / 2 + 16, 0, w / 2 - 16, 36, BLACK);
        M5.Display.setTextColor(WHITE, BLACK);
        M5.Display.setFont(&fonts::Font0);
        M5.Display.setTextSize(2);
        M5.Display.setCursor(w / 2 + 16, 0);
        M5.Display.printf("Score %04d", score);
        M5.Display.setCursor(w / 2 + 16, 22);
        M5.Display.printf("Lives %d", lives);
        scoreChanged = false;
    }
    if (challengeWordChanged) {
        if (gameState == GS_IDLE) {
            M5.Display.fillRect(0, 70, w, 96, BLACK);
            M5.Display.setTextColor(GREEN, BLACK);
            M5.Display.setFont(&fonts::Font2);
            String idleText1 = "TOUCH CENTER BUTTON";
            String idleText2 = "TO START";
            int ww1 = M5.Display.textWidth(idleText1);
            int ww2 = M5.Display.textWidth(idleText2);
            int hh = M5.Display.fontHeight(M5.Display.getFont());
            M5.Display.setCursor((w - ww1) / 2, (h - hh) / 2 - 10);
            M5.Display.print(idleText1);
            M5.Display.setCursor((w - ww2) / 2, (h - hh) / 2 + hh - 4);
            M5.Display.print(idleText2);
        }
        else if (gameState == GS_GAMEOVER) {
            M5.Display.fillRect(0, 70, w, 96, RED);
            M5.Display.setTextColor(WHITE, RED);
            M5.Display.setFont(&fonts::Font4);
            // M5.Display.setTextSize(1);
            String gameOverText = "GAME OVER";
            int ww = M5.Display.textWidth(gameOverText);
            int hh = M5.Display.fontHeight(M5.Display.getFont());
            M5.Display.setCursor((w - ww) / 2, (h - hh) / 2);
            M5.Display.print(gameOverText);
        }
        else {
            M5.Display.fillRect(0, 70, w, 96, BLACK);
            M5.Display.setTextColor(YELLOW, BLACK);
            M5.Display.setFont(&fonts::Font4);
            // M5.Display.setTextSize(1);
            int ww = M5.Display.textWidth(challengeWord);
            int hh = M5.Display.fontHeight(M5.Display.getFont());
            M5.Display.setCursor((w - ww) / 2, (h - hh) / 2);
            M5.Display.print(challengeWord);
        }
        challengeWordChanged = false;
    }
    if (remainingTimeChanged) {
        int dx = w * (remainingTime / 2.0) / timeout;
        M5.Display.drawLine(0, 160, w, 160, BLACK);
        M5.Display.drawLine(w / 2 - dx, 160, w / 2 + dx, 160, YELLOW);
        remainingTimeChanged = false;
    }
    M5.Display.endWrite();
}

void doGame() {
    if (gameState == GS_IDLE) {
        return;
    }
    else if (gameState == GS_INIT) {
        level = 1;
        timeout = 0;
        lives = 5;
        score = 0;
        challengeWord = "";
        inputWord = "";
        wordCompleted = false;
        successCount = 0;
        scoreChanged = true;
        screenNeedsUpdate = true;
        gameState = GS_PLAYING;
    }
    else if (gameState == GS_PLAYING) {
        // a word gap after the input was already judged or cleared ends
        // an empty word; ignore it
        if (wordCompleted && inputWord.equals("")) {
            wordCompleted = false;
        }
        if (challengeWord.equals("")) {
            if (level == 1) {
                challengeWord = morsePlain[random(0, 26)];
                timeout = 4000;
            }
            else if (level == 2) {
                challengeWord = words2[random(0, sizeof(words2) / sizeof(words2[0]))];
                timeout = 7000;
            }
            else if (level == 3) {
                challengeWord = words3[random(0, sizeof(words3) / sizeof(words3[0]))];
                timeout = 10000;
            }
            else if (level == 4) {
                challengeWord = words4[random(0, sizeof(words4) / sizeof(words4[0]))];
                timeout = 12000;
            }
            else {
                challengeWord = words5[random(0, sizeof(words5) / sizeof(words5[0]))];
                timeout = 15000;
            }
            startTime = millis();
            challengeWordChanged = true;
            screenNeedsUpdate = true;
            // draw the full bar right away
            lastTimeBarUpdate = startTime - TIME_BAR_INTERVAL_MS;
        }
        else if ((wordCompleted || inputWord.equals(challengeWord)) &&
                 millis() < startTime + timeout) {
            // judge a correct word as soon as its last character is
            // decoded; a wrong one still waits for the word gap
            if (challengeWord.equals(inputWord)) {
                M5.Speaker.tone(1600, 100, EFFECT_CH);
                score += level * 10;
                successCount++;
                if (level < 5 && successCount >= 10) {
                    level++;
                    successCount = 0;
                }
                challengeWord = "";
                scoreChanged = true;
                challengeWordChanged = true;
                screenNeedsUpdate = true;
            }
            else {
                M5.Speaker.tone(1000, 50, EFFECT_CH);
                M5.Speaker.tone(800, 50, EFFECT_CH, false);
            }
            inputWord = "";
            wordCompleted = false;
        }
        else if (millis() >= startTime + timeout) {
            lives--;
            M5.Speaker.tone(800, 500, EFFECT_CH);
            M5.Speaker.tone(700, 500, EFFECT_CH, false);
            if (lives <= 0) {
                gameState = GS_GAMEOVER;
                gameoverTimer = millis() + 5000;
                challengeWordChanged = true;
            }
            else {
                challengeWord = "";
            }
            inputWord = "";
            wordCompleted = false;
            scoreChanged = true;
            screenNeedsUpdate = true;
        }
        if (millis() - lastTimeBarUpdate >= TIME_BAR_INTERVAL_MS) {
            remainingTime = max(0L, (long)timeout - (long)(millis() - startTime));
            remainingTimeChanged = true;
            screenNeedsUpdate = true;
            lastTimeBarUpdate = millis();
        }
    }
    else if (gameState == GS_GAMEOVER) {
        if (millis() >= gameoverTimer) {
            gameState = GS_IDLE;
            challengeWordChanged = true;
            screenNeedsUpdate = true;
        }
    }
}

void iter() {
    // key state check
    // a change is accepted once the pin has been stable for minDuration;
    // its time is when the pin settled, so the delay cancels out in the
    // mark and gap lengths
    int keyState = digitalRead(KEY_PIN);
    if (keyState != rawKeyState) {
        rawKeyState = keyState;
        rawChangeTime = millis();
    }
    if (keyState != lastKeyState) {
        unsigned long changeTime = rawChangeTime;
        if (millis() - changeTime >= minDuration) {
            if (keyState == LOW) {
                stateNow = 1;
                M5.Speaker.tone(TONE_FREQ, UINT32_MAX, SIDETONE_CH);
                keyOn = true;
                screenNeedsUpdate = true;
                // Serial.println("Key Pressed");
            }
            else if (keyState == HIGH) {
                lastMarkLength = changeTime - lastChangeTime;
                lastMarkType = classifyMark(lastMarkLength);  // 0 dot, 1 dash
                newDataFlag = 1;
                stateNow = 0;
                M5.Speaker.stop(SIDETONE_CH);
                keyOn = false;
                screenNeedsUpdate = true;
                // Serial.println("Key Released");
            }
            lastKeyState = keyState;
            lastChangeTime = changeTime;
        }
    }

    if (stateNow == 0) {
        // constructing characters
        if (newDataFlag == 1) {
            // marks beyond the buffer are dropped; the character is then
            // unmatched because no code uses all 9 slots
            if (bufferPosition < sizeof(currentCharacter)) {
                if (lastMarkType == 0) {
                    // Serial.print(".");
                    currentCharacter[bufferPosition] = 1;
                }
                else {
                    // Serial.print("-");
                    currentCharacter[bufferPosition] = 2;
                }
                bufferPosition++;
            }
            newDataFlag = 0;
        }
        // end of charactoers
        bool matchFound = 0;
        if (millis() - lastChangeTime >= minCharacterGap &&
            bufferPosition != 0) {
            for (byte c = 0; c < 45; c++) {
                if (currentCharacter[0] == morseCode[c][0] &&
                    currentCharacter[1] == morseCode[c][1] &&
                    currentCharacter[2] == morseCode[c][2] &&
                    currentCharacter[3] == morseCode[c][3] &&
                    currentCharacter[4] == morseCode[c][4] &&
                    currentCharacter[5] == morseCode[c][5] &&
                    currentCharacter[6] == morseCode[c][6] &&
                    currentCharacter[7] == morseCode[c][7] &&
                    currentCharacter[8] == morseCode[c][8]) {
                    currentCharacterNumber = c;
                    matchFound = 1;
                    break;
                }
            }
            if (matchFound != 1) {
                currentCharacterNumber = 38;
                inputWord += "*";
                inputText += "*";
            }
            else {
                String s = morsePlain[currentCharacterNumber];
                // Serial.print(s);
                inputWord += s;
                inputText += s;
            }
            if (inputText.length() > maxInputText) {
                inputText.remove(0, inputText.length() - maxInputText);
            }
            inputTextChanged = true;
            screenNeedsUpdate = true;
            spaceFlag = 1;
            for (byte b = 0; b < 9; b++) {
                currentCharacter[b] = 0;
            }
            bufferPosition = 0;
        }
        // end of words
        if (millis() - lastChangeTime >= minWordGap && spaceFlag == 1) {
            // Serial.print(" ");
            inputText += " ";
            wordCompleted = true;
            // Serial.println(inputWord);
            if (inputText.length() > maxInputText) {
                inputText.remove(0, inputText.length() - maxInputText);
            }
            inputTextChanged = true;
            screenNeedsUpdate = true;
            spaceFlag = 0;
        }
    } // if (stateNow == 0)
    doGame();
}

void setup() {
    auto cfg = M5.config();
    cfg.internal_spk = true;
    cfg.serial_baudrate = 115200;
    M5.begin(cfg);
    pinMode(KEY_PIN, INPUT_PULLUP);
    w = M5.Display.width();
    h = M5.Display.height();
    M5.Display.fillScreen(BLACK);
    M5.Display.setFont(&fonts::Font2);
    M5.Display.setTextColor(CYAN, BLACK);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(0, 0);
    M5.Display.print("Morse Game");

    M5.Speaker.setVolume(220);
    // while (!Serial) delay(10);
    lastIterTime = millis();
    setTiming();
    setDrawPosition();
    gameState = GS_IDLE;
    scoreChanged = true;
    challengeWordChanged = true;
    inputTextChanged = true;
    screenNeedsUpdate = true;
}

void loop() {
    unsigned long t = millis();
    M5.update();
    if (M5.BtnB.wasPressed()) {
        if (gameState == GS_IDLE) {
            gameState = GS_INIT;
        }
    }
    if (t - lastIterTime >= ITER_INTERVAL_MS) {
        iter();
        lastIterTime = t;
    }
    if (screenNeedsUpdate) {
        drawScreen();
        screenNeedsUpdate = false;
    }
}
