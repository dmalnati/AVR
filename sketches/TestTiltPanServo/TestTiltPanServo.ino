#include "Evm.h"
#include "ServoController.h"


static Evm::Instance<10,10,10> evm;

static ServoController scX(15);
static ServoController scY(16);


void MoveToStartingPosition()
{
    Serial.println("MoveToStartingPosition");
    
    scX.MoveTo(80);
    scY.MoveTo(20);

    evm.HoldStackDangerously(500);
}

void MoveToTouchScreen()
{
    scY.MoveTo(32);
    
    evm.HoldStackDangerously(500);
}

void SwipeRight()
{
    scX.MoveTo(45);
    
    evm.HoldStackDangerously(500);
}


void setup()
{
    Serial.begin(9600);

    while (1)
    {
        MoveToStartingPosition();
        MoveToTouchScreen();
        SwipeRight();
    }
}

void loop() {}










