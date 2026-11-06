// Board configuration and loop timing.
#pragma once

// The key goes on the two signal pins of a Grove port. KEY_PIN takes a
// straight key, or the DIT paddle; KEY_PIN2 the DAH paddle (they can be
// swapped in the settings).
#if !defined(KEY_PIN)
#if defined(ARDUINO_M5STACK_Core2)
// M5Stack Core2 Grove Port A
#define KEY_PIN 33
#define KEY_PIN2 32
#elif defined(ARDUINO_M5STACK_FIRE)
// M5Stack Fire Grove Port B
#define KEY_PIN 36
#define KEY_PIN2 26
#else
// M5Stack CoreS3 Grove Port B
#define KEY_PIN 8
#define KEY_PIN2 9
#endif
#endif
#if !defined(KEY_PIN2)
#error "define KEY_PIN2 (the DAH paddle) together with KEY_PIN"
#endif

// The key is sampled and the game advanced every ITER_INTERVAL_MS.
#define ITER_INTERVAL_MS 4
