

class Profiler
{
    static const uint8_t LOOP_COUNT = 100;
    
public:
    Profiler(const char *str)
    : str_(str)
    {
        Start();
    }

    ~Profiler()
    {
        End();

        double usPerOp = ((double)(timeEnd_ - timeStart_) / (double)(LOOP_COUNT * 50));
        char *usPerOpStr = GetFStr(usPerOp);

        char buf[100];
        sprintf(buf, "%10s: %6li (at %i ops: %7s us/op)",
                str_,
                timeEnd_ - timeStart_,
                LOOP_COUNT * 50,
                usPerOpStr);
        
        @fix@Serial.println(buf);

        delay(500);
    }

private:
    inline void Start() { timeStart_ = micros(); }
    inline void End()   { timeEnd_   = micros(); }

    char *GetFStr(double d)
    {
        static char buf[50];

        dtostrf(d, 0, 3, buf);

        return buf;
    }

    const char *str_ = NULL;
    
    uint32_t timeStart_ = 0;
    uint32_t timeEnd_   = 0;
};

template <typename T>
class ProfilerIncr
: public Profiler
{
public:
    ProfilerIncr(const char *str)
    : Profiler(str)
    {
        T v1 = 0;

        for (uint8_t i = 0; i < LOOP_COUNT; ++i)
        {
            v1++;  v1++;  v1++;  v1++;  v1++;
            v1++;  v1++;  v1++;  v1++;  v1++;
            v1++;  v1++;  v1++;  v1++;  v1++;
            v1++;  v1++;  v1++;  v1++;  v1++;
            v1++;  v1++;  v1++;  v1++;  v1++;
            v1++;  v1++;  v1++;  v1++;  v1++;
            v1++;  v1++;  v1++;  v1++;  v1++;
            v1++;  v1++;  v1++;  v1++;  v1++;
            v1++;  v1++;  v1++;  v1++;  v1++;
            v1++;  v1++;  v1++;  v1++;  v1++;
        }

        @fix@Serial.print(v1);
        @fix@Serial.print('\r');
    }
};

template <typename T>
class ProfilerMult
: public Profiler
{
public:
    ProfilerMult(const char *str)
    : Profiler(str)
    {
        T v1 = 123;

        for (uint8_t i = 0; i < LOOP_COUNT; ++i)
        {
            v1 *= v1;  v1 *= v1;  v1 *= v1;  v1 *= v1;  v1 *= v1;
            v1 *= v1;  v1 *= v1;  v1 *= v1;  v1 *= v1;  v1 *= v1;
            v1 *= v1;  v1 *= v1;  v1 *= v1;  v1 *= v1;  v1 *= v1;
            v1 *= v1;  v1 *= v1;  v1 *= v1;  v1 *= v1;  v1 *= v1;
            v1 *= v1;  v1 *= v1;  v1 *= v1;  v1 *= v1;  v1 *= v1;
            v1 *= v1;  v1 *= v1;  v1 *= v1;  v1 *= v1;  v1 *= v1;
            v1 *= v1;  v1 *= v1;  v1 *= v1;  v1 *= v1;  v1 *= v1;
            v1 *= v1;  v1 *= v1;  v1 *= v1;  v1 *= v1;  v1 *= v1;
            v1 *= v1;  v1 *= v1;  v1 *= v1;  v1 *= v1;  v1 *= v1;
            v1 *= v1;  v1 *= v1;  v1 *= v1;  v1 *= v1;  v1 *= v1;
        }

        @fix@Serial.print(v1);
        @fix@Serial.print('\r');
    }
};

template <typename T>
class ProfilerDiv
: public Profiler
{
public:
    ProfilerDiv(const char *str)
    : Profiler(str)
    {
        T v1 = 123;

        for (uint8_t i = 0; i < LOOP_COUNT; ++i)
        {
            v1 /= v1;  v1 /= v1;  v1 /= v1;  v1 /= v1;  v1 /= v1;
            v1 /= v1;  v1 /= v1;  v1 /= v1;  v1 /= v1;  v1 /= v1;
            v1 /= v1;  v1 /= v1;  v1 /= v1;  v1 /= v1;  v1 /= v1;
            v1 /= v1;  v1 /= v1;  v1 /= v1;  v1 /= v1;  v1 /= v1;
            v1 /= v1;  v1 /= v1;  v1 /= v1;  v1 /= v1;  v1 /= v1;
            v1 /= v1;  v1 /= v1;  v1 /= v1;  v1 /= v1;  v1 /= v1;
            v1 /= v1;  v1 /= v1;  v1 /= v1;  v1 /= v1;  v1 /= v1;
            v1 /= v1;  v1 /= v1;  v1 /= v1;  v1 /= v1;  v1 /= v1;
            v1 /= v1;  v1 /= v1;  v1 /= v1;  v1 /= v1;  v1 /= v1;
            v1 /= v1;  v1 /= v1;  v1 /= v1;  v1 /= v1;  v1 /= v1;
        }

        @fix@Serial.print(v1);
        @fix@Serial.print('\r');
    }
};


template <typename T, uint8_t BITS>
class ProfilerShiftLeft
: public Profiler
{
public:
    ProfilerShiftLeft(const char *str)
    : Profiler(str)
    {
        T v1 = 123;

        for (uint8_t i = 0; i < LOOP_COUNT; ++i)
        {
            v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;
            v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;
            v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;
            v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;
            v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;
            v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;
            v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;
            v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;
            v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;
            v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;  v1 <<= BITS;
        }

        @fix@Serial.print(v1);
        @fix@Serial.print('\r');
    }
};

template <typename T, uint8_t BITS>
class ProfilerShiftRight
: public Profiler
{
public:
    ProfilerShiftRight(const char *str)
    : Profiler(str)
    {
        T v1 = 123;

        for (uint8_t i = 0; i < LOOP_COUNT; ++i)
        {
            v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;
            v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;
            v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;
            v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;
            v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;
            v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;
            v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;
            v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;
            v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;
            v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;  v1 >>= BITS;
        }

        @fix@Serial.print(v1);
        @fix@Serial.print('\r');
    }
};



/*
 * Compiled with no optimizations (-O0), so code size blows up.
 * Have to run some tests one by one.
 */

#define DO_INCR              0
#define DO_MULT              0
#define DO_DIV               0
#define DO_SHIFT_LEFT_8      0
#define DO_SHIFT_LEFT_16     0
#define DO_SHIFT_LEFT_32_1   0
#define DO_SHIFT_LEFT_32_2   0
#define DO_SHIFT_LEFT_32_3   0
#define DO_SHIFT_LEFT_32_4   0
#define DO_SHIFT_RIGHT_8     0
#define DO_SHIFT_RIGHT_16    0
#define DO_SHIFT_RIGHT_32_1  1
#define DO_SHIFT_RIGHT_32_2  0
#define DO_SHIFT_RIGHT_32_3  0
#define DO_SHIFT_RIGHT_32_4  0



void setup()
{
    @fix@Serial.begin(9600);

    while (1)
    {

#if DO_INCR == 1
        @fix@Serial.print("Incr"); @fix@Serial.println();
        @fix@Serial.print("----------"); @fix@Serial.println();
        ProfilerIncr<uint8_t>{"uint8_t"};
        ProfilerIncr<int8_t>{"int8_t"};
        ProfilerIncr<uint16_t>{"uint16_t"};
        ProfilerIncr<int16_t>{"int16_t"};
        ProfilerIncr<uint32_t>{"uint32_t"};
        ProfilerIncr<int32_t>{"int32_t"};
        ProfilerIncr<float>{"float"};
        ProfilerIncr<double>{"double"};
        @fix@Serial.println();
#endif

#if DO_MULT == 1
        @fix@Serial.print("Mult"); @fix@Serial.println();
        @fix@Serial.print("----------"); @fix@Serial.println();
        ProfilerMult<uint8_t>{"uint8_t"};
        ProfilerMult<int8_t>{"int8_t"};
        ProfilerMult<uint16_t>{"uint16_t"};
        ProfilerMult<int16_t>{"int16_t"};
        ProfilerMult<uint32_t>{"uint32_t"};
        ProfilerMult<int32_t>{"int32_t"};
        ProfilerMult<float>{"float"};
        ProfilerMult<double>{"double"};
        @fix@Serial.println();
#endif

#if DO_DIV == 1
        @fix@Serial.print("Div"); @fix@Serial.println();
        @fix@Serial.print("----------"); @fix@Serial.println();
        ProfilerDiv<uint8_t>{"uint8_t"};
        ProfilerDiv<int8_t>{"int8_t"};
        ProfilerDiv<uint16_t>{"uint16_t"};
        ProfilerDiv<int16_t>{"int16_t"};
        ProfilerDiv<uint32_t>{"uint32_t"};
        ProfilerDiv<int32_t>{"int32_t"};
        ProfilerDiv<float>{"float"};
        ProfilerDiv<double>{"double"};
        @fix@Serial.println();
#endif









#if DO_SHIFT_LEFT_8 == 1
        @fix@Serial.print("Shift Left 8"); @fix@Serial.println();
        @fix@Serial.print("----------"); @fix@Serial.println();
        
        ProfilerShiftLeft<uint8_t, 0>{"uint8_t << 0"};
        ProfilerShiftLeft<uint8_t, 1>{"uint8_t << 1"};
        ProfilerShiftLeft<uint8_t, 2>{"uint8_t << 2"};
        ProfilerShiftLeft<uint8_t, 3>{"uint8_t << 3"};
        ProfilerShiftLeft<uint8_t, 4>{"uint8_t << 4"};
        ProfilerShiftLeft<uint8_t, 5>{"uint8_t << 5"};
        ProfilerShiftLeft<uint8_t, 6>{"uint8_t << 6"};
        ProfilerShiftLeft<uint8_t, 7>{"uint8_t << 7"};
        ProfilerShiftLeft<uint8_t, 8>{"uint8_t << 8"};

        @fix@Serial.println();
#endif

#if DO_SHIFT_LEFT_16 == 1
        @fix@Serial.print("Shift Left 16"); @fix@Serial.println();
        @fix@Serial.print("----------"); @fix@Serial.println();
        
        ProfilerShiftLeft<uint16_t, 0>{"uint16_t << 0"};
        ProfilerShiftLeft<uint16_t, 1>{"uint16_t << 1"};
        ProfilerShiftLeft<uint16_t, 2>{"uint16_t << 2"};
        ProfilerShiftLeft<uint16_t, 3>{"uint16_t << 3"};
        ProfilerShiftLeft<uint16_t, 4>{"uint16_t << 4"};
        ProfilerShiftLeft<uint16_t, 5>{"uint16_t << 5"};
        ProfilerShiftLeft<uint16_t, 6>{"uint16_t << 6"};
        ProfilerShiftLeft<uint16_t, 7>{"uint16_t << 7"};
        ProfilerShiftLeft<uint16_t, 8>{"uint16_t << 8"};
        ProfilerShiftLeft<uint16_t, 9>{"uint16_t << 9"};
        ProfilerShiftLeft<uint16_t, 10>{"uint16_t << 10"};
        ProfilerShiftLeft<uint16_t, 11>{"uint16_t << 11"};
        ProfilerShiftLeft<uint16_t, 12>{"uint16_t << 12"};
        ProfilerShiftLeft<uint16_t, 13>{"uint16_t << 13"};
        ProfilerShiftLeft<uint16_t, 14>{"uint16_t << 14"};
        ProfilerShiftLeft<uint16_t, 15>{"uint16_t << 15"};
        ProfilerShiftLeft<uint16_t, 16>{"uint16_t << 16"};

        @fix@Serial.println();
#endif

#if DO_SHIFT_LEFT_32_1 == 1
        @fix@Serial.print("Shift Left 32 1"); @fix@Serial.println();
        @fix@Serial.print("----------"); @fix@Serial.println();
        
        ProfilerShiftLeft<uint32_t, 0>{"uint32_t << 0"};
        ProfilerShiftLeft<uint32_t, 1>{"uint32_t << 1"};
        ProfilerShiftLeft<uint32_t, 2>{"uint32_t << 2"};
        ProfilerShiftLeft<uint32_t, 3>{"uint32_t << 3"};
        ProfilerShiftLeft<uint32_t, 4>{"uint32_t << 4"};
        ProfilerShiftLeft<uint32_t, 5>{"uint32_t << 5"};
        ProfilerShiftLeft<uint32_t, 6>{"uint32_t << 6"};
        ProfilerShiftLeft<uint32_t, 7>{"uint32_t << 7"};
        ProfilerShiftLeft<uint32_t, 8>{"uint32_t << 8"};
        
        @fix@Serial.println();

#endif

#if DO_SHIFT_LEFT_32_2 == 1
        @fix@Serial.print("Shift Left 32 2"); @fix@Serial.println();
        @fix@Serial.print("----------"); @fix@Serial.println();
        
        ProfilerShiftLeft<uint32_t, 9>{"uint32_t << 9"};
        ProfilerShiftLeft<uint32_t, 10>{"uint32_t << 10"};
        ProfilerShiftLeft<uint32_t, 11>{"uint32_t << 11"};
        ProfilerShiftLeft<uint32_t, 12>{"uint32_t << 12"};
        ProfilerShiftLeft<uint32_t, 13>{"uint32_t << 13"};
        ProfilerShiftLeft<uint32_t, 14>{"uint32_t << 14"};
        ProfilerShiftLeft<uint32_t, 15>{"uint32_t << 15"};
        ProfilerShiftLeft<uint32_t, 16>{"uint32_t << 16"};

        @fix@Serial.println();
#endif

#if DO_SHIFT_LEFT_32_3 == 1
        @fix@Serial.print("Shift Left 32 3"); @fix@Serial.println();
        @fix@Serial.print("----------"); @fix@Serial.println();
        
        ProfilerShiftLeft<uint32_t, 17>{"uint32_t << 17"};
        ProfilerShiftLeft<uint32_t, 18>{"uint32_t << 18"};
        ProfilerShiftLeft<uint32_t, 19>{"uint32_t << 19"};
        ProfilerShiftLeft<uint32_t, 20>{"uint32_t << 20"};
        ProfilerShiftLeft<uint32_t, 21>{"uint32_t << 21"};
        ProfilerShiftLeft<uint32_t, 22>{"uint32_t << 22"};
        ProfilerShiftLeft<uint32_t, 23>{"uint32_t << 23"};
        ProfilerShiftLeft<uint32_t, 24>{"uint32_t << 24"};
        
        @fix@Serial.println();
#endif

#if DO_SHIFT_LEFT_32_4 == 1
        @fix@Serial.print("Shift Left 32 4"); @fix@Serial.println();
        @fix@Serial.print("----------"); @fix@Serial.println();
        
        ProfilerShiftLeft<uint32_t, 25>{"uint32_t << 25"};
        ProfilerShiftLeft<uint32_t, 26>{"uint32_t << 26"};
        ProfilerShiftLeft<uint32_t, 27>{"uint32_t << 27"};
        ProfilerShiftLeft<uint32_t, 28>{"uint32_t << 28"};
        ProfilerShiftLeft<uint32_t, 29>{"uint32_t << 29"};
        ProfilerShiftLeft<uint32_t, 30>{"uint32_t << 30"};
        ProfilerShiftLeft<uint32_t, 31>{"uint32_t << 31"};
        ProfilerShiftLeft<uint32_t, 32>{"uint32_t << 32"};

        @fix@Serial.println();
#endif
























#if DO_SHIFT_RIGHT_8 == 1
        @fix@Serial.print("Shift Right 8"); @fix@Serial.println();
        @fix@Serial.print("----------"); @fix@Serial.println();
        
        ProfilerShiftRight<uint8_t, 0>{"uint8_t >> 0"};
        ProfilerShiftRight<uint8_t, 1>{"uint8_t >> 1"};
        ProfilerShiftRight<uint8_t, 2>{"uint8_t >> 2"};
        ProfilerShiftRight<uint8_t, 3>{"uint8_t >> 3"};
        ProfilerShiftRight<uint8_t, 4>{"uint8_t >> 4"};
        ProfilerShiftRight<uint8_t, 5>{"uint8_t >> 5"};
        ProfilerShiftRight<uint8_t, 6>{"uint8_t >> 6"};
        ProfilerShiftRight<uint8_t, 7>{"uint8_t >> 7"};
        ProfilerShiftRight<uint8_t, 8>{"uint8_t >> 8"};
        
        @fix@Serial.println();
#endif


#if DO_SHIFT_RIGHT_16 == 1
        @fix@Serial.print("Shift Right 16"); @fix@Serial.println();
        @fix@Serial.print("----------"); @fix@Serial.println();
        
        ProfilerShiftRight<uint16_t, 0>{"uint16_t >> 0"};
        ProfilerShiftRight<uint16_t, 1>{"uint16_t >> 1"};
        ProfilerShiftRight<uint16_t, 2>{"uint16_t >> 2"};
        ProfilerShiftRight<uint16_t, 3>{"uint16_t >> 3"};
        ProfilerShiftRight<uint16_t, 4>{"uint16_t >> 4"};
        ProfilerShiftRight<uint16_t, 5>{"uint16_t >> 5"};
        ProfilerShiftRight<uint16_t, 6>{"uint16_t >> 6"};
        ProfilerShiftRight<uint16_t, 7>{"uint16_t >> 7"};
        ProfilerShiftRight<uint16_t, 8>{"uint16_t >> 8"};
        ProfilerShiftRight<uint16_t, 9>{"uint16_t >> 9"};
        ProfilerShiftRight<uint16_t, 10>{"uint16_t >> 10"};
        ProfilerShiftRight<uint16_t, 11>{"uint16_t >> 11"};
        ProfilerShiftRight<uint16_t, 12>{"uint16_t >> 12"};
        ProfilerShiftRight<uint16_t, 13>{"uint16_t >> 13"};
        ProfilerShiftRight<uint16_t, 14>{"uint16_t >> 14"};
        ProfilerShiftRight<uint16_t, 15>{"uint16_t >> 15"};
        ProfilerShiftRight<uint16_t, 16>{"uint16_t >> 16"};

        @fix@Serial.println();
#endif




#if DO_SHIFT_RIGHT_32_1 == 1
        @fix@Serial.print("Shift Right 32 1"); @fix@Serial.println();
        @fix@Serial.print("----------"); @fix@Serial.println();
        
        ProfilerShiftRight<uint32_t, 0>{"uint32_t >> 0"};
        ProfilerShiftRight<uint32_t, 1>{"uint32_t >> 1"};
        ProfilerShiftRight<uint32_t, 2>{"uint32_t >> 2"};
        ProfilerShiftRight<uint32_t, 3>{"uint32_t >> 3"};
        ProfilerShiftRight<uint32_t, 4>{"uint32_t >> 4"};
        ProfilerShiftRight<uint32_t, 5>{"uint32_t >> 5"};
        ProfilerShiftRight<uint32_t, 6>{"uint32_t >> 6"};
        ProfilerShiftRight<uint32_t, 7>{"uint32_t >> 7"};
        ProfilerShiftRight<uint32_t, 8>{"uint32_t >> 8"};
        
        @fix@Serial.println();

#endif

#if DO_SHIFT_RIGHT_32_2 == 1
        @fix@Serial.print("Shift Right 32 2"); @fix@Serial.println();
        @fix@Serial.print("----------"); @fix@Serial.println();
        
        ProfilerShiftRight<uint32_t, 9>{"uint32_t >> 9"};
        ProfilerShiftRight<uint32_t, 10>{"uint32_t >> 10"};
        ProfilerShiftRight<uint32_t, 11>{"uint32_t >> 11"};
        ProfilerShiftRight<uint32_t, 12>{"uint32_t >> 12"};
        ProfilerShiftRight<uint32_t, 13>{"uint32_t >> 13"};
        ProfilerShiftRight<uint32_t, 14>{"uint32_t >> 14"};
        ProfilerShiftRight<uint32_t, 15>{"uint32_t >> 15"};
        ProfilerShiftRight<uint32_t, 16>{"uint32_t >> 16"};

        @fix@Serial.println();
#endif

#if DO_SHIFT_RIGHT_32_3 == 1
        @fix@Serial.print("Shift Right 32 3"); @fix@Serial.println();
        @fix@Serial.print("----------"); @fix@Serial.println();
        
        ProfilerShiftRight<uint32_t, 17>{"uint32_t >> 17"};
        ProfilerShiftRight<uint32_t, 18>{"uint32_t >> 18"};
        ProfilerShiftRight<uint32_t, 19>{"uint32_t >> 19"};
        ProfilerShiftRight<uint32_t, 20>{"uint32_t >> 20"};
        ProfilerShiftRight<uint32_t, 21>{"uint32_t >> 21"};
        ProfilerShiftRight<uint32_t, 22>{"uint32_t >> 22"};
        ProfilerShiftRight<uint32_t, 23>{"uint32_t >> 23"};
        ProfilerShiftRight<uint32_t, 24>{"uint32_t >> 24"};
        
        @fix@Serial.println();
#endif

#if DO_SHIFT_RIGHT_32_4 == 1
        @fix@Serial.print("Shift Right 32 4"); @fix@Serial.println();
        @fix@Serial.print("----------"); @fix@Serial.println();
        
        ProfilerShiftRight<uint32_t, 25>{"uint32_t >> 25"};
        ProfilerShiftRight<uint32_t, 26>{"uint32_t >> 26"};
        ProfilerShiftRight<uint32_t, 27>{"uint32_t >> 27"};
        ProfilerShiftRight<uint32_t, 28>{"uint32_t >> 28"};
        ProfilerShiftRight<uint32_t, 29>{"uint32_t >> 29"};
        ProfilerShiftRight<uint32_t, 30>{"uint32_t >> 30"};
        ProfilerShiftRight<uint32_t, 31>{"uint32_t >> 31"};
        ProfilerShiftRight<uint32_t, 32>{"uint32_t >> 32"};

        @fix@Serial.println();
#endif











        delay(2000);
    }
}

void loop() {}












