#include "color_parser.h"
#include <cstring>
#include <cstdio>

void parseHexColor(const char* hex, float& r, float& g, float& b) {
    if (!hex || hex[0] != '#' || strlen(hex) < 7) {
        r = g = b = 1.0f;
        return;
    }
    unsigned int ri, gi, bi;
    sscanf(hex + 1, "%02x%02x%02x", &ri, &gi, &bi);
    r = ri / 255.0f;
    g = gi / 255.0f;
    b = bi / 255.0f;
}
