#include "Evm.h"
#include "LCDFrentaly20x4.h"
#include "Keypad12Button.h"
#include "ServoController.h"



static const uint8_t PIN_SERVO = 25;


class KeypadMonitor
{
public:
    KeypadMonitor()
    : kpad_({
        10, 11, 12, 13, 14, 15, 16, 17, 18, 19
    })
    , sc_(PIN_SERVO)
    {
        ClearBuf();
    }

    void Run()
    {
        // Set up LCD
        lcd_.Init();

        lcd_.PrintAt(0,  0, "MoveTo: ");
        lcd_.PrintAt(0,  1, "Last  : ");

        // Set up Keypad
        kpad_.Init([&](char c) {
            OnKeyPress(c);
        });

        // Handle events
        evm_.MainLoop();
    }

private:

    static const uint8_t CHAR_BUF_SIZE = 3;

    void OnKeyPress(char c)
    {
        switch (c)
        {
            case '*': ClearBuf();    break;
            case '#': MoveServo();   break;
            default : AppendChar(c); break;
        }

        PrintBuf();
    }

    void PrintBuf()
    {
        uint8_t COL = 8;
        uint8_t ROW = 0;

        lcd_.PrintAt(COL, ROW, (const char *)degreeBuf_);
    }

    void PrintLast()
    {
        uint8_t COL = 8;
        uint8_t ROW = 1;

        lcd_.PrintAt(COL, ROW, (const char *)degreeBuf_);
    }

    void ClearBuf()
    {
        for (uint8_t i = 0; i < CHAR_BUF_SIZE; ++i)
        {
            degreeBuf_[i] = ' ';
        }
        degreeBuf_[CHAR_BUF_SIZE] = '\0';

        degreeBufIdx_ = 0;

        PrintBuf();
    }

    uint8_t GetNumFromBuf()
    {
        uint16_t num = 0;

        for (uint8_t i = 0; i < CHAR_BUF_SIZE && degreeBuf_[i] != ' '; ++i)
        {
            num = (num * 10) + (degreeBuf_[i] - '0');
        }

        if (num > 180)
        {
            num = 180;
        }

        return (uint8_t)num;
    }

    void MoveServo()
    {
        uint8_t num = GetNumFromBuf();
        
        sc_.MoveTo(num);

        PrintLast();
        
        ClearBuf();
    }

    void AppendChar(char c)
    {
        if (degreeBufIdx_ < CHAR_BUF_SIZE)
        {
            degreeBuf_[degreeBufIdx_] = c;

            ++degreeBufIdx_;
        }
    }

    Evm::Instance<10,10,10> evm_;
    
    LCDFrentaly20x4 lcd_;
    Keypad12Button  kpad_;
    ServoController sc_;

    uint8_t degreeBuf_[CHAR_BUF_SIZE + 1];
    uint8_t degreeBufIdx_;
};





void setup()
{
    KeypadMonitor km;    // wasn't working before in static global scope... why?
    km.Run();
}

void loop() {}






