# MORSE GAME for M5Stack

This game is designed to help you improve your Morse code transmission skills.
It has been tested on the M5Stack Core2 and the M5Stack CoreS3-SE.

When the game begins, an alphabet letter or an English word will appear on the screen. Use your Morse key to enter the correct Morse code before the time bar beneath the prompt runs out. If you send the code correctly, you move on to the next challenge. If you make a mistake, you can try again as long as there is still time remaining. Failing to send the correct code before the timer expires results in a 1-point penalty. Accumulating 5 penalty points ends the game.

All you need is a Morse key, which you can connect to a GPIO pin such as a Grove connector.

This game was created by Takuo Watanabe for an exhibition at the Yokohama Midorigaoka High School Homecoming Day held on November 15, 2025.

## Building

The sketch is built with [arduino-cli](https://arduino.github.io/arduino-cli/).
`sketch.yaml` defines one profile per target board; the ESP32 core and the
libraries are installed automatically on the first build.

| Profile | Board | Morse key pin |
|---|---|---|
| `core2` | M5Stack Core2 | GPIO 33 (Grove Port A) |
| `core3` | M5Stack CoreS3 / CoreS3-SE (default) | GPIO 8 (Grove Port B) |

Build artifacts are kept per profile under `build/<profile>`:

```sh
arduino-cli compile --profile core2 --build-path build/core2
arduino-cli upload  --profile core2 --build-path build/core2 -p <serial-port>
```

The ESP32 core is pinned to 2.0.17. With core 3.x, M5Unified's speaker task
overflows its stack and the device crashes on the first sound.
