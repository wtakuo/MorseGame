// Runs the firmware's straight key decoder (MorseDecoder.cpp, MorseCode.cpp)
// on a PC, for tools/morse_sim.py.
//
//   c++ -std=c++17 -O2 -I.. -o decoder_host decoder_host.cpp ../MorseDecoder.cpp ../MorseCode.cpp
//
// Input (stdin): "<initial dot ms>" on the first line, then one key edge per
// line as "<time ms> <level>" (level 0 = pressed, 1 = released), then a line
// "end <time ms>". Several such blocks may follow each other; each one is
// decoded from a fresh state.
// Output (stdout): the decoded words of each block on one line, separated by
// spaces.
#include <stdio.h>
#include <string.h>

#include <string>
#include <utility>
#include <vector>

#include "../Config.h"
#include "../MorseDecoder.h"

static std::string decode(float dotMs, const std::vector<std::pair<double, int>> &edges,
                          double end) {
    StraightKey key;
    SpeedTracker speed(dotMs);
    CharacterDecoder decoder;
    std::string out, word;
    size_t i = 0;
    int level = 1;
    // sample like loop() / iter(): every ITER_INTERVAL_MS
    for (uint32_t now = 0; now < end + 20.0 * speed.wordGap(); now += ITER_INTERVAL_MS) {
        while (i < edges.size() && edges[i].first <= now) {
            level = edges[i++].second;
        }
        uint32_t lastRelease = key.lastChangeTime();
        if (key.update(now, level == 0)) {
            if (key.isDown()) {
                speed.gap(key.lastChangeTime() - lastRelease);
            }
            else {
                decoder.addMark(speed.classify(key.lastMarkLength()));
            }
        }
        if (!key.isDown()) {
            uint32_t gap = now - key.lastChangeTime();
            if (const char *text = decoder.checkCharacterEnd(gap, speed.charGap())) {
                word += text;
            }
            if (decoder.checkWordEnd(gap, speed.wordGap())) {
                out += (out.empty() ? "" : " ") + word;
                word.clear();
            }
        }
    }
    if (!word.empty()) {
        out += (out.empty() ? "" : " ") + word;
    }
    return out;
}

int main() {
    char line[128];
    while (fgets(line, sizeof(line), stdin)) {
        float dotMs;
        if (sscanf(line, "%f", &dotMs) != 1) {
            continue;
        }
        std::vector<std::pair<double, int>> edges;
        double end = 0;
        while (fgets(line, sizeof(line), stdin)) {
            double t;
            int level;
            if (sscanf(line, "end %lf", &end) == 1) {
                break;
            }
            if (sscanf(line, "%lf %d", &t, &level) == 2) {
                edges.emplace_back(t, level);
            }
        }
        printf("%s\n", decode(dotMs, edges, end).c_str());
    }
    return 0;
}
