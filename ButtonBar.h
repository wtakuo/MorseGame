// Labels of the three buttons (left, centre, right) along the bottom of
// the screen. On a Core2 they sit above its touch buttons below the
// screen; on a CoreS3 the labels themselves are the touch buttons.
#pragma once

namespace ButtonBar {

const int HEIGHT = 36;

// Icons drawn as triangles instead of text.
extern const char *const DOWN;
extern const char *const LEFT;
extern const char *const RIGHT;

// Make the bottom HEIGHT pixels of a CoreS3 touch screen act as BtnA-C.
void begin();

// Draw the labels; nullptr leaves a button blank.
void draw(const char *left, const char *centre, const char *right);

}  // namespace ButtonBar
