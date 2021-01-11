// debug.h

// macros for turning debugging statements on and off
// (and maybe later for redirecting to other outputs...)

#define DEBUG

#ifdef DEBUG
#   define PRINT(...)   Serial.print(__VA_ARGS__)
#   define PRINTLN(...) Serial.println(__VA_ARGS__)
#   define PRINTF(...)  Serial.printf(__VA_ARGS__)
#else
#   define PRINT(...)
#   define PRINTLN(...)
#   define PRINTF(...)
#endif

#if defined DEBUG
#   define BEGIN(speed, debug) Serial.begin((speed)); while (!Serial) {}; Serial.setDebugOutput(debug)
#else
#   define BEGIN(speed, debug)
#endif
