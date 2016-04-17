#include "Function.h"
#include "PAL.h"


// Do some measurements on size

template <uint8_t SIZE>
class VarSizeFunctor
{
public:
    uint8_t operator()(uint8_t val)
    {
        return buf_[0] + SIZE + val;
    }

private:
    uint8_t buf_[SIZE ? SIZE : 1];
};

static function<int(double)>      f_int__double;
static function<double(bool,int)> f_double__bool_int;

static volatile int ref1 = rand();
static volatile int ref2 = rand();
static volatile int ref3 = rand();
static volatile int ref4 = rand();
static volatile int ref5 = rand();
static function<int(int)>            f_two_captures = [&](int val) -> int { return ref1 = ref2 = val; };
static function<int(int)>            f_three_captures = [&](int val) -> int { return ref1 = ref2 = ref3 = val; };
static function<int(int)>            f_four_captures = [&](int val) -> int { return ref1 = ref2 = ref3 = ref4 = val; };
static function<int(int)>            f_five_captures = [&](int val) -> int { return ref1 = ref2 = ref3 = ref4 = ref5 = val; };

auto l = [&](int val) -> int { return ref1 = ref2 = ref3 = ref4 = ref5 = val; };
static FunctorCarrierConcrete<typeof(l), int, int> fcc(l);
static VarSizeFunctor<sizeof(l)> measure_l;
static function<uint8_t(uint8_t)> f_measure_l = measure_l;

// I think these are being optimized away or something, should have broke by now.


class ObjLambdaUser
{
public:
    ObjLambdaUser(uint8_t seed)
    : seed_(seed)
    {
        // Nothing to do
    }

    function<uint8_t()> GetCallback()
    {
        return [this]() { return this->DoSomething(); };
    }

private:
    uint8_t DoSomething()
    {
        return seed_ + rand();
    }

    uint8_t seed_;
};

static ObjLambdaUser olu(rand());
static function<uint8_t()> f_olu = olu.GetCallback();




static VarSizeFunctor<1> vsf1;
static VarSizeFunctor<2> vsf2;
static VarSizeFunctor<3> vsf3;
static VarSizeFunctor<4> vsf4;
static VarSizeFunctor<5> vsf5;
static VarSizeFunctor<6> vsf6;
static VarSizeFunctor<7> vsf7;

static function<uint8_t(uint8_t)> f_test = [=](uint8_t val) { return vsf7(val); };


static function<uint8_t(uint8_t)>        f_under_limit = VarSizeFunctor<6>{};

// This actually exceeds the limit
//static function<uint8_t(uint8_t)>        f_over_limit = VarSizeFunctor<7>{};




// Compare performance
const uint8_t PIN_TO_TOGGLE = 15;
uint8_t       PIN_STATE     = 0;

void RawFunctionPinToggle()
{
    PIN_STATE = !PIN_STATE;

    PAL.DigitalWrite(PIN_TO_TOGGLE, PIN_STATE);
}

class SimpleFunctor
{
public:
    void operator()()
    {
        PIN_STATE = !PIN_STATE;
    
        PAL.DigitalWrite(PIN_TO_TOGGLE, PIN_STATE);
    }
};

static function<void()> fSimpleFunctor = SimpleFunctor{};


static function<void()> fLambda = []() -> void {
    PIN_STATE = !PIN_STATE;
    
    PAL.DigitalWrite(PIN_TO_TOGGLE, PIN_STATE);
};



void setup()
{
    // Set up output pin for use
    PAL.PinMode(PIN_TO_TOGGLE, OUTPUT);
    PAL.DigitalWrite(PIN_TO_TOGGLE, PIN_STATE);

    // Invoke functions
    f_two_captures(rand());
    f_three_captures(rand());
    f_four_captures(rand());
    f_five_captures(rand());
    f_test(rand());
    f_olu();
}

void loop()
{
    for (uint8_t i = 0; i < 20; ++i)
    {
        RawFunctionPinToggle();
    }

    PAL.Delay(1);

    for (uint8_t i = 0; i < 20; ++i)
    {
        fSimpleFunctor();
    }

    PAL.Delay(1);

    for (uint8_t i = 0; i < 20; ++i)
    {
        fLambda();
    }

    PAL.Delay(1);
}





