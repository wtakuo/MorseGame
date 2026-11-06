# MORSE GAME for M5Stack

[日本語](README.ja.md)

Version 0.1.0

This game is designed to help you improve your Morse code transmission skills.
It has been tested on the M5Stack Core2 and the M5Stack CoreS3-SE.

When the game begins, an alphabet letter or an English word will appear on the screen. Use your Morse key to enter the correct Morse code before the time bar beneath the prompt runs out. If you send the code correctly, you move on to the next challenge. If you make a mistake, you can try again as long as there is still time remaining. Failing to send the correct code before the timer expires results in a 1-point penalty. Accumulating 5 penalty points (3 on the HARD level) ends the game. Every 10 correct answers (5 on EASY) take you up a stage, from single characters (stage 1) to five-letter words (stage 5).

All you need is a Morse key: a straight key, or a paddle with the built-in iambic keyer, connected to a Grove port.

## Key wiring

| Board | Port | Straight key / DIT paddle | DAH paddle |
|---|---|---|---|
| M5Stack Core2 | Grove Port A | GPIO 33 | GPIO 32 |
| M5Stack CoreS3 / CoreS3-SE | Grove Port B | GPIO 8 | GPIO 9 |

The contacts close to GND (the pins use internal pull-ups). The DIT and DAH paddles can be swapped in the settings.

## Controls

Three buttons, labelled along the bottom of the screen. On the Core2 they are the touch buttons below the screen; on the CoreS3 the labels themselves are touch buttons.

| Screen | Left | Centre | Right |
|---|---|---|---|
| Title | Settings | Start | |
| Playing | | Pause | |
| Paused | Quit | Resume | |
| Settings | Next item (▼) | Previous value (◀) | Next value (▶) |

Settings:

| Item | Values |
|---|---|
| KEY | STRAIGHT, IAMBIC-A, IAMBIC-B |
| SPEED | 5-40 WPM: the keyer speed, or the starting speed for a straight key, which then follows the player |
| DIT / DAH | NORMAL, REVERSED (swaps the paddles) |
| LEVEL | EASY, NORMAL, HARD |
| VOLUME | 0-10 |
| RESET | settings back to the defaults (press twice) |
| CLEAR SCORES | high scores of all levels (press twice) |
| EXIT | save and return |

The levels:

| | EASY | NORMAL | HARD |
|---|---|---|---|
| Time limit (stages 1-5) | 8/12/16/20/24 s | 4/7/10/12/15 s | 3/5/7/9/11 s |
| Correct answers to go up a stage | 5 | 10 | 10 |
| Lives | 5 | 5 | 3 |
| Stage 1 asks | letters | letters and digits | letters and digits |

The settings and a high score for each level are kept in flash. The battery level is shown at the top right of the settings screen ("+" while charging).

This game was created for an exhibition at the Yokohama Midorigaoka High School Homecoming Day held on November 15, 2025.

## Building

The sketch is built with [arduino-cli](https://arduino.github.io/arduino-cli/).
`sketch.yaml` defines one profile per target board; the ESP32 core and the
libraries are installed automatically on the first build.

| Profile | Board | Key pins |
|---|---|---|
| `core2` | M5Stack Core2 | GPIO 33, 32 (Grove Port A) |
| `cores3` | M5Stack CoreS3 / CoreS3-SE (default) | GPIO 8, 9 (Grove Port B) |

Build artifacts are kept per profile under `build/<profile>`:

```sh
arduino-cli compile --profile core2 --build-path build/core2
arduino-cli upload  --profile core2 --build-path build/core2 -p <serial-port>
```

The ESP32 core is pinned to 2.0.17. With core 3.x, M5Unified's speaker task
overflows its stack and the device crashes on the first sound.
