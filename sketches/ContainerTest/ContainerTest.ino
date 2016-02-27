#include <Container.h>


template <typename T>
class SQCMP
{
public:
    uint8_t operator()(T, T) { return 0; }
};


template <uint8_t THINGS_COUNT>
class ThingHaver
{
public:

private:
    RingBuffer<uint8_t, THINGS_COUNT>                   rb_;
    Queue<uint8_t, THINGS_COUNT>                        q_;
    SortedQueue<uint8_t, THINGS_COUNT, SQCMP<uint8_t> > sq_;
    ListInPlace<uint8_t, THINGS_COUNT>                  lip_;
    
};

RingBuffer<uint8_t, 10> rb;
Queue<uint8_t, 10> q;
SortedQueue<uint8_t, 10, SQCMP<uint8_t> > sq;
ListInPlace<uint8_t, 10> lip;

ThingHaver<1>  th1;
ThingHaver<2>  th2;
ThingHaver<5>  th5;
ThingHaver<10> th10;


void setup()
{

}


void loop()
{
    
}


