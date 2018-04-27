#include <stdbool.h>

#include "print.h"
#include "logging.h"

static bool show_header;
static int interval;
static long lasttick;

void LoggingInit(void)
{
    show_header = false;
    interval = 0;
}

void LoggingOn(int intervalMs)
{
    show_header = true;
    interval = intervalMs;
}

void LoggingOff(void)
{
    show_header = false;
    interval = 0;
}

void LoggingUpdate(unsigned long micros, float cur_set, float current, float voltage, float power, float charge, float energy)
{
    unsigned long tick, millis;
    
    if (show_header) {
        print("\n");
        //print("T(sec), Is(A), I(A), V(V), P(W), Q(mAh), E(mWh)\n");

        print("%8s, %6s, %6s, %6s, %6s, %6s, %7s\n",
              "T(sec)", "Is(A)", "I(A)", "V(V)", "P(W)", "Q(mAh)", "E(mWh)");
        show_header = false;
    }

    millis = micros / 1000;
    if (interval > 0) {
        tick = millis / interval;
        if (tick != lasttick) {

            char buf[15];
            sprint(buf, 15, "%.1f", millis / 1000.0);
            
            print("%-8s, %6.3f, %6.3f, %6.3f, %6.3f, %6.3f, %7.3f\n",
                buf, cur_set, current, voltage, power, charge / 3.6, energy / 3.6);
            lasttick = tick;
        }
    }
}


