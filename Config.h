// Board configuration and loop timing.
#pragma once

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

// The key is sampled and the game advanced every ITER_INTERVAL_MS.
#define ITER_INTERVAL_MS 4
