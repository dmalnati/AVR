// Macros for serial debugging Arduino sketches
#ifndef __DEBUGMACROS_H__
#define __DEBUGMACROS_H__

static const char EMPTY[] = ""; 

#define DEBUG 0
#define debug_print(msg, ...) \
  do { if (DEBUG) { @fix@Serial.print(__func__); @fix@Serial.print("::"); \
    @fix@Serial.print(msg, ##__VA_ARGS__); } } while (0)
// prints a short version w/o the function label
#define debug_prints(msg, ...) \
  do { if (DEBUG) @fix@Serial.print(msg, ##__VA_ARGS__); } while (0)
#define debug_println(msg, ...) \
  do { if (DEBUG) { @fix@Serial.print(__func__); @fix@Serial.print("::"); \
    @fix@Serial.println(msg, ##__VA_ARGS__); } } while (0)
// prints a short version w/o the function label
#define debug_printlns(msg, ...) \
  do { if (DEBUG) @fix@Serial.println(msg, ##__VA_ARGS__); } while (0)

#endif
