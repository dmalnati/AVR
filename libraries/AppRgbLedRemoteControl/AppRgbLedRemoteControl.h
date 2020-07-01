#ifndef __APP_RGB_LED_REMOTE_CONTROL_H__
#define __APP_RGB_LED_REMOTE_CONTROL_H__


#include "PAL.h"
#include "Log.h"
#include "Evm.h"
#include "SerialInput.h"
#include "RFLink.h"





/*

Logic of master:
- self-set programmed pattern
- startup
  - send out over radio on regular basis
  - perform yourself
- get serial command
  - update pre-programmed pattern
  - send out over radio
  - perform yourself



Logic of slave:
- 
- get radio command
  - set






*/





////////////////////////////////////////////////////////////////////////////////
//
// RGB LED PWM Control Core
//
////////////////////////////////////////////////////////////////////////////////







// Should be a class for simply controlling PWM on RGB
// Should be a class for controlling fading, etc, using that other class.
// Should be a class deciding what to do with those









class RgbLedEffectsController
{
public:
    RgbLedEffectsController()
    {
        // Nothing to do
    }

    void Start()
    {
        ted_.SetCallback([this](){
            OnTimeout();
        });

        ted_.RegisterForTimedEventIntervalRigid(10, 0);

        pwmController_.Init();
        pwmController_.Start();

        Log("Effects controller started");
    }

    ~RgbLedEffectsController()
    {
        ted_.DeRegisterForTimedEvent();
    }


private:

    void OnTimeout()
    {
        pwmController_.SetRGB(
            pwmController_.GetRed() + 3,
            pwmController_.GetGreen() + 3,
            pwmController_.GetBlue() + 3
        );

        // Log("Effects OnTimeout");
    }


    TimedEventHandlerDelegate ted_;

    RgbLedPwmController pwmController_;
};








////////////////////////////////////////////////////////////////////////////////
//
// Base
//
////////////////////////////////////////////////////////////////////////////////

// Basis of both master and slave is that they can be controlled via serial
// commands.
struct AppRgbLedRemoteControlBase
{
protected:

    struct Msg
    {
        uint8_t redVal;
        uint8_t blueVal;
        uint8_t greenVal;
    };

    static const uint16_t BAUD = 100;


public:
    AppRgbLedRemoteControlBase()
    {
        // Init();
    }


protected:


    void Init()
    {
        console_.RegisterCommand("start", 0, [this](char *){
            // what to do?
        });
        console_.RegisterCommand("set", 3, [this](char *){
            // what to do?
        });
        console_.RegisterCommand("stop", 0, [this](char *){
            // what to do?
        });

        console_.Start();

        rgbController_.Start();
    }


private:

    void OnControlEvent()
    {
        // Do nothing
    }

    SerialAsyncConsoleEnhanced<10> console_;

    RgbLedEffectsController rgbController_;
};




////////////////////////////////////////////////////////////////////////////////
//
// Common
//
////////////////////////////////////////////////////////////////////////////////


struct Config
{

};



////////////////////////////////////////////////////////////////////////////////
//
// Master
//
////////////////////////////////////////////////////////////////////////////////

class AppRgbLedRemoteControlMaster
: public AppRgbLedRemoteControlBase
{
public:
    AppRgbLedRemoteControlMaster(Config)
    {
        // Nothing to do
    }

    void Run()
    {
        LogStart(1200);
        Log(P("Starting Master"));


        SetUpRFLink();
        SetUpTiming();
        
        // should be above, but want to see what happens
        Init();
        
        evm_.MainLoop();
    }


private:

    void PrimeRF()
    {
        // uint8_t pin = 3;

        // Log("a");
        // for (uint8_t i = 0; i < 30; ++i)
        // {
        //     PAL.PinMode(pin, INPUT);
        //     //PAL.DigitalWrite(pin, LOW);
        //     PAL.Delay(1);
        //     PAL.PinMode(pin, INPUT_PULLUP);
        //     // PAL.DigitalWrite(pin, HIGH);
        //     PAL.Delay(1);
        // }
        // PAL.PinMode(pin, OUTPUT);
        // Log("b");

        LogNNL(5);
        LogNNL(5);
        LogNNL(5);
    }

    void OnTimeout()
    {
        PrimeRF();
        Log("OnTimeout");

        // Set up message
        Msg msg = {
            .redVal = 15,
            .blueVal = 16,
            .greenVal = 17,
        };


        // Send it
        uint32_t now = PAL.Micros();
        SendMsg(msg);
        uint32_t diff = PAL.Micros() - now;

        // Apply it yourself
        PrimeRF();
        Log("Sent it at ", PAL.Millis(), "; diff: ", diff);
    }

    template <typename T>
    void SendMsg(T msg)
    {
        rfLink_.Send((uint8_t *)&msg, sizeof(T));
    }

    void SetUpTiming()
    {
        ted_.SetCallback([this](){
            OnTimeout();
        });
        ted_.RegisterForTimedEventIntervalRigid(1000, 0);
    }

    void SetUpRFLink()
    {
        rfLink_.Init(15, 14, BAUD);

        rfLink_.SetRealm(0);
        rfLink_.SetSrcAddr(0);
        rfLink_.SetDstAddr(255);
        rfLink_.SetProtocolId(0);
    }


private:

    static const uint8_t C_IDLE  =  0;
    static const uint8_t C_TIMED = 20;
    static const uint8_t C_INTER =  0;
    
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    TimedEventHandlerDelegate ted_;

    RFLink rfLink_;
};


////////////////////////////////////////////////////////////////////////////////
//
// Slave
//
////////////////////////////////////////////////////////////////////////////////

class AppRgbLedRemoteControlSlave
: public AppRgbLedRemoteControlBase
{
public:
    AppRgbLedRemoteControlSlave(Config)
    {
        // Nothing to do
    }

    void Run()
    {
        LogStart(9600);
        Log(P("Starting Slave"));


        SetUpRFLink();
        SetUpTiming();
        
        // should be above, but want to see what happens
        //Init();
        
        evm_.MainLoop();
    }


private:

    void OnTimeout()
    {
        Log("Slave OnTimeout");
    }

    void SetUpTiming()
    {
    }

    void OnMessageReceived(RFLinkHeader *hdr, uint8_t *buf, uint8_t bufSize)
    {
        Msg &msg = *(Msg *)buf;

        Log("OnMessageReceived(");
        Log("[", hdr->realm, ", ", hdr->srcAddr, ", ", hdr->dstAddr, ", ", hdr->protocolId, "]");
        Log(msg.redVal, ", ", msg.blueVal, ", ", msg.greenVal);
    }

    void SetUpRFLink()
    {
        rfLink_.Init(15, -1, BAUD);

        rfLink_.SetOnMessageReceivedCallback([this](RFLinkHeader *hdr, uint8_t *buf, uint8_t bufSize){
            OnMessageReceived(hdr, buf, bufSize);
        });

        rfLink_.SetRealm(0);
        rfLink_.SetSrcAddr(255);
        rfLink_.SetDstAddr(0);
        rfLink_.SetProtocolId(0);
    }


private:

    static const uint8_t C_IDLE  =  0;
    static const uint8_t C_TIMED = 20;
    static const uint8_t C_INTER =  0;
    
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    TimedEventHandlerDelegate ted_;

    RFLink rfLink_;
};




















#endif  // __APP_RGB_LED_REMOTE_CONTROL_H__

















