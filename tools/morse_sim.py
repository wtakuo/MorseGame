#!/usr/bin/env python3
"""Keying simulator for MorseGame.ino.

Generates human-like straight-key input (speed, dash/dot ratio, jitter,
contact bounce), samples it every 4 ms with the same debounce as iter(),
and decodes it with interchangeable timing strategies so that changes to
the decoder can be compared before they go to the hardware.

    python3 tools/morse_sim.py               # compare the strategies
    python3 tools/morse_sim.py --check-host  # firmware decoder vs UnitGaps

--check-host builds tools/decoder_host.cpp with the firmware's
MorseDecoder.cpp and checks that it decodes like UnitGaps (needs c++).
"""
import difflib
import subprocess
import sys
import tempfile
import math
import os
import random
import re
import statistics

CODES = {
    'A': '.-', 'B': '-...', 'C': '-.-.', 'D': '-..', 'E': '.', 'F': '..-.',
    'G': '--.', 'H': '....', 'I': '..', 'J': '.---', 'K': '-.-', 'L': '.-..',
    'M': '--', 'N': '-.', 'O': '---', 'P': '.--.', 'Q': '--.-', 'R': '.-.',
    'S': '...', 'T': '-', 'U': '..-', 'V': '...-', 'W': '.--', 'X': '-..-',
    'Y': '-.--', 'Z': '--..',
}
# decode with the firmware's table (letters, digits and prosigns)
_TABLE = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'MorseCode.cpp')
with open(_TABLE) as _f:
    DECODE = {code: text for text, code in re.findall(r'\{"([^"]+)", "([.-]+)"\}', _f.read())}

WORDS = ('CAT DOG SUN RUN SIT HIS USE ITS SIX AIR '
         'THIS SHIP FISH WISH BEST LIST SAID HEAT '
         'HOUSE STILL SHORT HAPPY MUSIC WATER SPACE CLASS').split()

ITER_MS = 4
DEBOUNCE_MS = 10


# ---------------------------------------------------------------- keyer

class Keyer:
    """A human keying with a straight key."""

    def __init__(self, wpm, ratio=3.0, jitter=0.1, last_hold=1.0,
                 dot_scale=1.0, gap_scale=1.0, bounce=True):
        self.unit = 1200.0 / wpm
        self.ratio = ratio          # dash length / dot length
        self.jitter = jitter        # sigma of log-normal length noise
        self.last_hold = last_hold  # last mark of a character is held longer
        self.dot_scale = dot_scale  # dot length relative to the unit
        self.gap_scale = gap_scale  # gap lengths relative to the unit
        self.bounce = bounce

    def edges(self, words, rnd, word_pause=1500, long_press_before=()):
        """Return [(t_ms, level)], level 0 = pressed, and the end time.

        Words are separated by a word gap plus `word_pause` ms, the time a
        player takes to read the next challenge.
        """
        t, out = 500.0, []

        def noisy(x):
            return x * math.exp(rnd.gauss(0, self.jitter))

        def chatter(level):
            nonlocal t
            if self.bounce:
                for _ in range(rnd.randint(0, 3)):
                    out.append((t, level)); t += rnd.uniform(0.2, 1.5)
                    out.append((t, 1 - level)); t += rnd.uniform(0.2, 1.5)

        u = self.unit
        for wi, word in enumerate(words):
            if wi:
                t += noisy(7 * self.unit) + word_pause * math.exp(rnd.gauss(0, 0.2))
            if wi in long_press_before:
                out.append((t, 0)); t += 2000; out.append((t, 1))
                t += noisy(7 * self.unit) + word_pause
            for ci, ch in enumerate(word):
                if ci:
                    t += noisy(3 * u * self.gap_scale)
                code = CODES[ch]
                for mi, m in enumerate(code):
                    if mi:
                        t += noisy(u * self.gap_scale)
                    length = u * (self.dot_scale if m == '.' else self.ratio)
                    if mi == len(code) - 1:
                        length *= self.last_hold
                    chatter(0)
                    out.append((t, 0))
                    t += noisy(length)
                    chatter(1)
                    out.append((t, 1))
        return out, t


# ------------------------------------------------------------ strategies

class Current:
    """The decoder as of commit c7d7553."""
    name = 'current'

    def __init__(self, dot=60):
        self.dot = dot
        self.update()

    def update(self):
        self.max_dot = 1.7 * self.dot
        self.char_gap = 1.8 * self.dot
        self.word_gap = 6.0 * self.dot
        self.last_dot = self.dot

    def mark(self, length):
        is_dash = length >= self.max_dot
        if is_dash:
            length //= 3
        self.dot = (9 * self.last_dot + length) // 10
        self.update()
        return is_dash


class TwoMeans:
    """Track dot and dash lengths separately; the threshold is between them.

    alpha:     weight of a new mark in the running averages
    ratio:     allowed dash/dot ratio range; the other average is pulled
               into it so that a run of one kind still moves both
    dot_range: limits of the dot length (ms)
    split:     position of the threshold between the averages (0.5 = mid)
    """

    def __init__(self, dot=60, alpha=0.25, ratio=(2.0, 4.0),
                 dot_range=(25, 300), split=0.5, char_gap=2.0, name=None):
        self.avg_dot = dot
        self.avg_dash = 3 * dot
        self.alpha = alpha
        self.ratio = ratio
        self.dot_range = dot_range
        self.split = split
        self.char_gap_k = char_gap
        self.name = name or f'two-means a={alpha} split={split}'
        self.update()

    def update(self):
        lo, hi = self.ratio
        self.avg_dash = min(max(self.avg_dash, lo * self.avg_dot), hi * self.avg_dot)
        self.avg_dot = min(max(self.avg_dot, self.avg_dash / hi), self.avg_dash / lo)
        self.avg_dot = min(max(self.avg_dot, self.dot_range[0]), self.dot_range[1])
        self.avg_dash = min(max(self.avg_dash, lo * self.avg_dot), hi * self.avg_dot)
        # truncated to whole ms like the int thresholds in the firmware
        self.max_dot = int(self.avg_dot + self.split * (self.avg_dash - self.avg_dot))
        self.char_gap = int(self.char_gap_k * self.avg_dot)
        self.word_gap = int(6.0 * self.avg_dot)

    def mark(self, length):
        is_dash = length >= self.max_dot
        a = self.alpha
        if is_dash:
            self.avg_dash += a * (length - self.avg_dash)
        else:
            self.avg_dot += a * (length - self.avg_dot)
        self.update()
        return is_dash


class WindowMeans(TwoMeans):
    """The decoder of c83c557-0645018: TwoMeans, plus a window of the last
    `window` marks.

    When the window holds both short and long marks (longest/shortest >=
    `bimodal`), the dot and dash averages are taken directly from a
    two-cluster split of the window, so the estimate snaps to the player's
    speed and dash/dot ratio as soon as a character mixes dots and dashes.
    Otherwise it falls back to the running averages of TwoMeans.
    """

    def __init__(self, dot=80, window=8, bimodal=2.0, outlier=1000, name=None, **kw):
        super().__init__(dot=dot, **kw)
        self.window = window
        self.bimodal = bimodal
        self.hist = []
        self.outlier = outlier
        self.name = name or f'window n={window} a={self.alpha} split={self.split}'

    def mark(self, length):
        # a long press (resting on the key) says nothing about the speed
        if length > max(3 * self.avg_dash, self.outlier):
            return True
        self.hist = (self.hist + [length])[-self.window:]
        lo, hi = min(self.hist), max(self.hist)
        if hi >= self.bimodal * lo:
            th = math.sqrt(lo * hi)
            for _ in range(4):
                short = [x for x in self.hist if x < th]
                long_ = [x for x in self.hist if x >= th]
                th = (statistics.mean(short) + statistics.mean(long_)) / 2
            self.avg_dot = statistics.mean(short)
            self.avg_dash = statistics.mean(long_)
            self.update()
            return length >= self.max_dot
        return super().mark(length)


class UnitGaps(WindowMeans):
    """The decoder in MorseDecoder.cpp (SpeedTracker): WindowMeans with a
    wider dash/dot ratio, and the character and word gaps measured in a
    unit taken from the dashes (1/3) and the gaps within characters rather
    than from the dots, which some players flick very short."""

    MIN_DOT = 25

    def __init__(self, dot=80, name=None, **kw):
        self.avg_gap = dot
        kw.setdefault('ratio', (2.0, 8.0))
        super().__init__(dot=dot, name=name or 'unit-gaps', **kw)

    def update(self):
        super().update()
        self.unit = (self.avg_dash / 3 + self.avg_gap) / 2
        self.char_gap = int(2.0 * self.unit)
        self.word_gap = int(6.0 * self.unit)

    def gap(self, length):
        if length < self.MIN_DOT or length >= self.char_gap:
            return
        self.avg_gap += self.alpha * (length - self.avg_gap)
        self.avg_gap = min(max(self.avg_gap, self.dot_range[0]), self.dot_range[1])
        self.update()


# --------------------------------------------------------------- decoder

def decode(edges, end, strategy):
    """Sample the key every 4 ms like iter() and return the decoded words."""
    level_at, i = 1, 0
    raw, raw_t, last, last_t = 1, 0, 1, 0
    buf, word, words, space = '', '', [], False
    t = 0
    while t < end + 20 * strategy.word_gap:
        while i < len(edges) and edges[i][0] <= t:
            level_at = edges[i][1]; i += 1
        if level_at != raw:
            raw, raw_t = level_at, t
        if level_at != last and t - raw_t >= DEBOUNCE_MS:
            if level_at == 1:
                buf += '-' if strategy.mark(raw_t - last_t) else '.'
            elif hasattr(strategy, 'gap'):
                strategy.gap(raw_t - last_t)
            last, last_t = level_at, raw_t
        if last == 1:
            if buf and t - last_t >= strategy.char_gap:
                word += DECODE.get(buf, '*')
                buf, space = '', True
            if space and t - last_t >= strategy.word_gap:
                words.append(word)
                word, space = '', False
        t += ITER_MS
    if word:
        words.append(word)
    return words


# ------------------------------------------------------------ evaluation

PROFILES = {
    # name: Keyer keyword arguments other than wpm
    'clean':      dict(ratio=3.0, jitter=0.10),
    'last-hold':  dict(ratio=3.0, jitter=0.12, last_hold=1.4),
    'typical':    dict(ratio=2.6, jitter=0.15, last_hold=1.2),
    'short-dash': dict(ratio=2.3, jitter=0.15),
    # like a player logged on a CoreS3: very short flicked dots, short gaps
    'flick':      dict(ratio=3.0, jitter=0.2, dot_scale=0.45, gap_scale=0.8),
}


def score(keyer, strategy_factory, trials=30, n_words=12, warmup=2):
    """Word accuracy over the first `warmup` words and over the rest."""
    first = later = n_first = n_later = 0
    for s in range(trials):
        rnd = random.Random(s)
        words = [rnd.choice(WORDS) for _ in range(n_words)]
        edges, end = keyer.edges(words, rnd)
        got = decode(edges, end, strategy_factory())
        # align so that a split or merged word does not shift the rest
        hit = set()
        for blk in difflib.SequenceMatcher(None, words, got, autojunk=False).get_matching_blocks():
            hit.update(range(blk.a, blk.a + blk.size))
        for k, w in enumerate(words):
            ok = k in hit
            if k < warmup:
                first += ok; n_first += 1
            else:
                later += ok; n_later += 1
    return first / n_first, later / n_later


def confusion(keyer, strategy_factory, trials=200):
    """Rate of S->U and I->A style errors (last dot read as a dash)."""
    counts = {}
    for target in ('S', 'I', 'H', 'E'):
        bad = 0
        for s in range(trials):
            rnd = random.Random(1000 + s)
            # a few warm-up letters so the speed estimate has settled
            words = [rnd.choice(WORDS) for _ in range(5)] + [target]
            edges, end = keyer.edges(words, rnd)
            got = decode(edges, end, strategy_factory())
            bad += not (got and got[-1] == target)
        counts[target] = bad / trials
    return counts


def scenario_long_press(strategy_factory, wpm=12, trials=30):
    """Accuracy of the 3 words after a 2 s press (key held by mistake)."""
    ok = n = 0
    for s in range(trials):
        rnd = random.Random(2000 + s)
        words = [rnd.choice(WORDS) for _ in range(9)]
        k = Keyer(wpm, **PROFILES['typical'])
        edges, end = k.edges(words, rnd, long_press_before=(6,))
        got = decode(edges, end, strategy_factory())
        got = [g for g in got if g not in ('T', '')]   # the long press itself
        hit = set()
        for blk in difflib.SequenceMatcher(None, words, got, autojunk=False).get_matching_blocks():
            hit.update(range(blk.a, blk.a + blk.size))
        ok += sum(k in hit for k in range(6, 9)); n += 3
    return ok / n


def scenario_switch(strategy_factory, fast=25, slow=8, trials=30):
    """A fast player is followed by a slow one (the estimate is kept).
    Returns the slow player's accuracy over their first 2 and next 6 words."""
    first = later = 0
    for s in range(trials):
        rnd = random.Random(3000 + s)
        strat = strategy_factory()
        w1 = [rnd.choice(WORDS) for _ in range(6)]
        e1, end1 = Keyer(fast, **PROFILES['typical']).edges(w1, rnd)
        decode(e1, end1, strat)
        w2 = [rnd.choice(WORDS) for _ in range(8)]
        e2, end2 = Keyer(slow, **PROFILES['typical']).edges(w2, rnd)
        got = decode(e2, end2, strat)
        hit = set()
        for blk in difflib.SequenceMatcher(None, w2, got, autojunk=False).get_matching_blocks():
            hit.update(range(blk.a, blk.a + blk.size))
        first += sum(k in hit for k in range(2)); later += sum(k in hit for k in range(2, 8))
    return first / (2 * trials), later / (6 * trials)


def main():
    strategies = {
        'current': lambda: Current(),
        'window': lambda: WindowMeans(),
        'unit': lambda: UnitGaps(),
    }
    print('Word accuracy: first 2 words (cold start) / later words; '
          'S, I = rate of S and I misread after 5 words')
    for prof, kw in PROFILES.items():
        print(f'## {prof} {kw}')
        for wpm in (6, 8, 10, 13, 16, 20, 25):
            cells = []
            for n, f in strategies.items():
                a, b = score(Keyer(wpm, **kw), f, trials=20)
                c = confusion(Keyer(wpm, **kw), f, trials=60)
                cells.append(f'{n:>7} {a:4.0%} / {b:4.0%}  S {c["S"]:3.0%} I {c["I"]:3.0%}')
            print(f'{wpm:3} WPM | ' + ' | '.join(cells))
        print()
    print('## scenarios (typical keying)')
    for n, f in strategies.items():
        lp = scenario_long_press(f)
        sw1 = scenario_switch(f, fast=25, slow=8)
        sw2 = scenario_switch(f, fast=8, slow=25)
        print(f'{n:>7}: after a 2 s press {lp:4.0%} | 25->8 WPM {sw1[0]:4.0%} / {sw1[1]:4.0%}'
              f' | 8->25 WPM {sw2[0]:4.0%} / {sw2[1]:4.0%}')


def check_host():
    """Decode the same traces with the firmware code and with UnitGaps."""
    here = os.path.dirname(os.path.abspath(__file__))
    with tempfile.TemporaryDirectory() as tmp:
        exe = os.path.join(tmp, 'decoder_host')
        subprocess.run(['c++', '-std=c++17', '-O2', '-I', os.path.join(here, '..'), '-o', exe,
                        os.path.join(here, 'decoder_host.cpp'),
                        os.path.join(here, '..', 'MorseDecoder.cpp'),
                        os.path.join(here, '..', 'MorseCode.cpp')], check=True)
        dot = UnitGaps().avg_dot
        cases, blocks = [], []
        for kw in PROFILES.values():
            for wpm in (6, 8, 10, 13, 16, 20, 25, 30):
                for s in range(25):
                    rnd = random.Random(s * 100 + wpm)
                    words = [rnd.choice(WORDS) for _ in range(8)]
                    edges, end = Keyer(wpm, **kw).edges(words, rnd)
                    cases.append((edges, end))
                    blocks.append(f'{dot}\n' + ''.join(f'{t:.3f} {lv}\n' for t, lv in edges)
                                  + f'end {end:.3f}\n')
        out = subprocess.run([exe], input=''.join(blocks), capture_output=True,
                             text=True, check=True).stdout.split('\n')
    diff = 0
    for (edges, end), fw in zip(cases, out):
        py = ' '.join(decode(edges, end, UnitGaps()))
        if py != fw:
            diff += 1
            if diff <= 5:
                print(f'  UnitGaps: {py}\n  firmware: {fw}')
    # float (firmware) and double (Python) can round a mark the other way
    # right at a threshold, so allow a few differences
    print(f'{diff} / {len(cases)} traces decoded differently')
    return diff <= len(cases) // 100


if __name__ == '__main__':
    if '--check-host' in sys.argv[1:]:
        sys.exit(0 if check_host() else 1)
    main()
