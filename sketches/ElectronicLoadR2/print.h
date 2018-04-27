/**
 * Initializes the Print sub-system.
 */
void PrintInit(void);

/**
 * Prints a formatted text the serial console.
 * 
 * @param[in] fmt printf-style format string
 */
void print(const char *fmt, ...);
void sprint(char *buf, int bufSize, const char *fmt, ...);

