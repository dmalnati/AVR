#include <Container.h>

template <typename T>
class SQCMP
{
public:
    uint8_t operator()(T, T) { return 0; }
};

RingBuffer<uint8_t, 10> rb;
Queue<uint8_t, 10> q;
SortedQueue<uint8_t, 10, SQCMP<uint8_t> > sq;
ListInPlace<uint8_t, 10> lip;


void setup()
{

}


void loop()
{
    
}


