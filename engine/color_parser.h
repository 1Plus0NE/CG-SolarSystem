#ifndef COLOR_PARSER_H
#define COLOR_PARSER_H

// Parse a "#RRGGBB" hex string into RGB floats in [0,1].
// Falls back to white (1,1,1) on malformed input.
void parseHexColor(const char* hex, float& r, float& g, float& b);

#endif // COLOR_PARSER_H
