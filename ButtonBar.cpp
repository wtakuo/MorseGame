#include "ButtonBar.h"

#include <M5Unified.h>

namespace ButtonBar {

const char *const DOWN = "\x01";
const char *const LEFT = "\x02";
const char *const RIGHT = "\x03";

void begin() {
    auto b = M5.getBoard();
    if (b == m5::board_t::board_M5StackCoreS3 || b == m5::board_t::board_M5StackCoreS3SE) {
        // M5Unified turns touches at y >= 240 - height into BtnA-C. With the
        // default height 0 only y >= 240 counts, which some CoreS3 panels
        // report at the bottom edge and others hardly ever.
        M5.setTouchButtonHeight(HEIGHT);
    }
}

void draw(const char *left, const char *centre, const char *right) {
    int w = M5.Display.width(), h = M5.Display.height();
    const char *labels[] = {left, centre, right};
    M5.Display.fillRect(0, h - HEIGHT, w, HEIGHT, BLACK);
    M5.Display.setFont(&fonts::Font2);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(WHITE, BLACK);
    int cy = h - HEIGHT / 2 + 1;  // centre line of the buttons
    for (int i = 0; i < 3; i++) {
        const char *label = labels[i];
        if (!label) {
            continue;
        }
        int x = w * i / 3 + 6, bw = w / 3 - 12, cx = x + bw / 2;
        M5.Display.drawRoundRect(x, h - HEIGHT + 4, bw, HEIGHT - 6, 6, DARKGREY);
        if (label == DOWN) {
            M5.Display.fillTriangle(cx, cy + 7, cx - 8, cy - 7, cx + 8, cy - 7, WHITE);
        }
        else if (label == LEFT) {
            M5.Display.fillTriangle(cx - 8, cy, cx + 6, cy - 8, cx + 6, cy + 8, WHITE);
        }
        else if (label == RIGHT) {
            M5.Display.fillTriangle(cx + 8, cy, cx - 6, cy - 8, cx - 6, cy + 8, WHITE);
        }
        else {
            M5.Display.setCursor(cx - M5.Display.textWidth(label) / 2, cy - 8);
            M5.Display.print(label);
        }
    }
}

}  // namespace ButtonBar
