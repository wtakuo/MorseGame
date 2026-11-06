// Tests for IambicKeyer (MorseDecoder.cpp) on a PC.
//
//   c++ -std=c++17 -I.. -o keyer_test keyer_test.cpp ../MorseDecoder.cpp ../MorseCode.cpp && ./keyer_test
//
// Each case holds the paddles over time intervals, runs the keyer every
// 4 ms like iter(), and checks the elements sent and their timing.
#include <stdio.h>

#include <string>
#include <vector>

#include "../MorseDecoder.h"

struct Hold {
    bool dah;           // which paddle
    uint32_t from, to;  // held during [from, to) ms
};

struct Result {
    std::string elements;    // e.g. ".-."
    std::string decoded;     // characters and " " at word ends
    std::vector<uint32_t> marks;  // mark lengths in ms
};

static Result run(IambicKeyer::Mode mode, uint32_t unit, const std::vector<Hold> &holds,
                  uint32_t end) {
    IambicKeyer keyer;
    keyer.configure(mode, unit);
    CharacterDecoder decoder;
    Result r;
    uint32_t markStart = 0;
    for (uint32_t now = 0; now < end; now += 4) {
        bool dit = false, dah = false;
        for (const Hold &h : holds) {
            if (now >= h.from && now < h.to) {
                (h.dah ? dah : dit) = true;
            }
        }
        if (keyer.update(now, dit, dah)) {
            if (keyer.isDown()) {
                markStart = keyer.lastChangeTime();
            }
            else {
                bool dash = keyer.lastElementDash();
                r.elements += dash ? '-' : '.';
                r.marks.push_back(keyer.lastChangeTime() - markStart);
                decoder.addMark(dash);
            }
        }
        if (!keyer.isDown()) {
            uint32_t gap = now - keyer.lastChangeTime();
            if (const char *t = decoder.checkCharacterEnd(gap, keyer.charGap())) {
                r.decoded += t;
            }
            if (decoder.checkWordEnd(gap, keyer.wordGap())) {
                r.decoded += ' ';
            }
        }
    }
    return r;
}

static int failures = 0;

static void expect(const char *name, const Result &r, const char *elements,
                   const char *decoded) {
    bool ok = r.elements == elements && r.decoded == decoded;
    printf("%s %-40s elements %-10s decoded \"%s\"\n", ok ? "ok  " : "FAIL", name,
           r.elements.c_str(), r.decoded.c_str());
    if (!ok) {
        printf("     expected elements %s decoded \"%s\"\n", elements, decoded);
        failures++;
    }
}

int main() {
    using M = IambicKeyer::Mode;
    const uint32_t u = 80;  // 15 WPM

    // a single tap sends one element
    expect("tap dit", run(M::A, u, {{false, 0, 20}}, 2000), ".", "E ");
    expect("tap dah", run(M::A, u, {{true, 0, 20}}, 2000), "-", "T ");

    // holding repeats: S = 3 dits, O = 3 dahs
    expect("hold dit for 3 dits", run(M::A, u, {{false, 0, 5 * u - 20}}, 3000), "...", "S ");
    expect("hold dah for 3 dahs", run(M::A, u, {{true, 0, 11 * u - 20}}, 3000), "---", "O ");

    // A = dit then dah: tap dit, then hold dah from inside the dit
    expect("dit then dah (A)", run(M::A, u, {{false, 0, 20}, {true, 40, 3 * u}}, 3000), ".-", "A ");

    // squeeze from a dit, release both during the 2nd element (dah, which
    // runs from 2u to 5u). Mode A stops: ".-" = A. Mode B adds a dit: ".-." = R
    std::vector<Hold> squeeze = {{false, 0, 3 * u}, {true, 0, 3 * u}};
    expect("squeeze, release in dah, mode A", run(M::A, u, squeeze, 3000), ".-", "A ");
    expect("squeeze, release in dah, mode B", run(M::B, u, squeeze, 3000), ".-.", "R ");

    // dah memory: tap the dah paddle briefly during a dit and release it
    // before the dit ends. Mode B remembers it, mode A does not.
    std::vector<Hold> tap = {{false, 0, 20}, {true, 30, 60}};
    expect("dah tapped during dit, mode A", run(M::A, u, tap, 3000), ".", "E ");
    expect("dah tapped during dit, mode B", run(M::B, u, tap, 3000), ".-", "A ");

    // timing: marks are exactly 1 and 3 units at element boundaries
    Result t = run(M::A, u, {{false, 0, 20}, {true, 40, 3 * u}}, 3000);
    bool timing = t.marks.size() == 2 && t.marks[0] == u && t.marks[1] == 3 * u;
    printf("%s %-40s marks %u, %u ms\n", timing ? "ok  " : "FAIL", "mark lengths",
           t.marks.size() > 0 ? t.marks[0] : 0, t.marks.size() > 1 ? t.marks[1] : 0);
    failures += !timing;

    // two characters and a word: "TE" = dah, gap, dit
    expect("character gap", run(M::A, u, {{true, 0, 20}, {false, 8 * u, 8 * u + 20}}, 3000),
           "-.", "TE ");

    printf("%d failure(s)\n", failures);
    return failures ? 1 : 0;
}
