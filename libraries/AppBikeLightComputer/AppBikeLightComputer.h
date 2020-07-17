#ifndef __APP_BIKE_LIGHT_COMPUTER_H__
#define __APP_BIKE_LIGHT_COMPUTER_H__


#include "Log.h"
#include "Evm.h"
#include "SerialInput.h"
#include "RFLink.h"
#include "RgbLedEffectsController.h"






struct MsgTxnDumpStatus
{
    static const uint8_t MSG_TYPE = 0;
    uint8_t msgType = MSG_TYPE;
};

struct MsgTxnApply
{
    static const uint8_t MSG_TYPE = 1;
    uint8_t msgType = MSG_TYPE;

    uint8_t alsoStart = 0;

    RgbLedEffectsController::RgbColorState rgbColorState;
};

struct MsgTxnStart
{
    static const uint8_t MSG_TYPE = 2;
    uint8_t msgType = MSG_TYPE;
};

struct MsgTxnPause
{
    static const uint8_t MSG_TYPE = 3;
    uint8_t msgType = MSG_TYPE;
};

struct MsgTxnStop
{
    static const uint8_t MSG_TYPE = 4;
    uint8_t msgType = MSG_TYPE;
};



// Compile-time warn about sizing
static_assert(sizeof(MsgTxnApply) <= RFLink::MAX_PACKET_SIZE, "MsgTxnStart too large");







struct AppBikeLightComputerConfig
{
    // interfacing config
    uint8_t pinConfigure;

    // radio config
    uint8_t pinIrq;
    uint8_t pinSdn;
    uint8_t pinSel;

    // initial settings
    uint32_t periodRedMs;
    uint32_t periodGreenMs;
    uint32_t periodBlueMs;
};


class AppBikeLightComputer
{
private:

    struct AppBikeLightComputerUserConfig
    {
        uint8_t addr  = 0;
        uint8_t realm = 0;
    };

    class AppBikeLightComputerConfigManager
    : public PersistantConfigManager<AppBikeLightComputerUserConfig, 2>
    {
    public:
        using PersistantConfigManager::PersistantConfigManager;
        
    private:
        virtual void SetupMenu() override
        {
            Menu().RegisterParamU8(P("addr"),   &Config().addr);
            Menu().RegisterParamU8(P("realm"),  &Config().realm);
        }
    };


public:
    AppBikeLightComputer(AppBikeLightComputerConfig &cfg)
    : cfg_(cfg)
    , radio_(cfg_.pinIrq, cfg_.pinSdn, cfg_.pinSel)
    {
        // Nothing to do
    }

    void Run()
    {
        LogStart(9600);
        Log(P("Starting"));

        // Get user config
        uint8_t userConfigOk = 0;
        
        {
            AppBikeLightComputerConfigManager mgr(cfg_.pinConfigure, userConfig_, 0);
            
            // For use in testing out different configurations
            uint8_t letDefaultApplyAutomatically = 1;
            userConfigOk = mgr.GetConfig(letDefaultApplyAutomatically);
        }

        if (userConfigOk)
        {
            Log(P("Config OK"));
            LogNL();

            SetUpEffects();
            SetUpRadio();

            evm_.MainLoop();
        }
        else
        {
            Log(P("Bad Configuration"));
        }
    }


protected:

    void OnMsg(const MsgTxnDumpStatus &)
    {
        radio_.DumpStatus();
    }

    void OnMsg(const MsgTxnApply &msg)
    {
        Log("Got TxnApply: ", msg.msgType);

        rgb_.SetState(msg.rgbColorState);

        if (msg.alsoStart)
        {
            rgb_.Start();
        }
    }

    void OnMsg(const MsgTxnStart &msg)
    {
        Log("Got TxnStart: ", msg.msgType);

        rgb_.Start();
    }

    void OnMsg(const MsgTxnPause &msg)
    {
        Log("Got TxnPause: ", msg.msgType);

        rgb_.Pause();
    }

    void OnMsg(const MsgTxnStop &msg)
    {
        Log("Got TxnStop: ", msg.msgType);

        rgb_.Stop();
    }


private:

    void SetUpEffects()
    {
        rgb_.SetPeriodRed(cfg_.periodRedMs);
        rgb_.SetPeriodGreen(cfg_.periodGreenMs);
        rgb_.SetPeriodBlue(cfg_.periodBlueMs);

        rgb_.Start();
    }

    template <typename T>
    uint8_t CheckAndDispatch(uint8_t *buf, uint8_t bufLen)
    {
        uint8_t retVal = 0;

        if (bufLen == sizeof(T) && buf[0] == T::MSG_TYPE)
        {
            retVal = 1;

            OnMsg(*(T *)buf);
        }

        return retVal;
    }

    void SetUpRadio()
    {
        radio_.Init();

        radio_.SetSrcAddr(userConfig_.addr);
        radio_.SetRealm(userConfig_.realm);
        radio_.SetReceiveBroadcast(1);

        radio_.SetOnMessageReceivedCallback([this](RFLinkHeader *, uint8_t *buf, uint8_t bufLen){
            CheckAndDispatch<MsgTxnDumpStatus>(buf, bufLen);
            CheckAndDispatch<MsgTxnApply>(buf, bufLen);
            CheckAndDispatch<MsgTxnStart>(buf, bufLen);
            CheckAndDispatch<MsgTxnPause>(buf, bufLen);
            CheckAndDispatch<MsgTxnStop>(buf, bufLen);
        });

        radio_.ModeReceive();
    }


private:

    static const uint8_t C_IDLE  =  0;
    static const uint8_t C_TIMED = 10;
    static const uint8_t C_INTER = 10;
    
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    AppBikeLightComputerConfig     &cfg_;
    AppBikeLightComputerUserConfig  userConfig_;

protected:

    RFLink radio_;

    RgbLedEffectsController rgb_;
};








struct AppBikeLightComputerRemoteConfig
{
    AppBikeLightComputerConfig cfg;
};




class AppBikeLightComputerRemote
: public AppBikeLightComputer
{
public:
    AppBikeLightComputerRemote(AppBikeLightComputerRemoteConfig &cfg)
    : AppBikeLightComputer(cfg.cfg)
    , cfg_(cfg)
    {
        // Nothing to do
    }

    void Run()
    {
        SetUpRadio();
        SetUpCommandHandler();
        InputsEnable();

        // Run inner class
        AppBikeLightComputer::Run();
    }

private:

    void SetUpRadio()
    {
        radio_.SetOnMessageTransmittedCallback([this](){
            Log(P("Msg Sent"));
            InputsEnable();
        });
    }

    void InputsEnable()
    {

    }

    void InputsDisable()
    {

    }


    template <typename T>
    void SendAndApply(const T &t)
    {
        // Send to remote systems
        radio_.Send((uint8_t *)&t, sizeof(t));

        // Wait for transmission to complete before allowing new ones
        InputsDisable();

        // Apply internally
        OnMsg(t);
    }

    void PrintColorState(RgbLedEffectsController::ColorState &cs)
    {
        Log(P("val             : "), cs.val);
        Log(P("periodMs        : "), cs.periodMs);
        Log(P("phaseOffsetBrads: "), cs.phaseOffsetBrads);
        Log(P("rotation        : "), cs.rotation);
    }

    void SetUpCommandHandler()
    {
        console_.RegisterCommand("apply", [this](char *cmdStr){
            Str str(cmdStr);

            MsgTxnApply msg;

            if (str.TokenCount(' ') == 2)
            {
                uint32_t val = atoi(str.TokenAtIdx(1, ' '));

                msg.alsoStart = val;
            }

            Log(P("Apply "), msg.alsoStart ? P("and Start") : P("but not Start"));

            msg.rgbColorState = rgb_.GetState();

            // Log(P("Red"));
            // PrintColorState(msg.rgbColorState.red);
            // Log(P("Green"));
            // PrintColorState(msg.rgbColorState.green);
            // Log(P("Blue"));
            // PrintColorState(msg.rgbColorState.blue);

            SendAndApply(msg);
        });

        console_.RegisterCommand("start", [this](char *){
            Log(P("Start"));

            MsgTxnStart msg;

            SendAndApply(msg);
        });

        console_.RegisterCommand("aas", [this](char *){
            Log(P("ApplyAndStart"));

            console_.Exec("apply 1");
        });

        console_.RegisterCommand("pause", [this](char *){
            Log(P("Pause"));

            MsgTxnPause msg;

            SendAndApply(msg);
        });
        
        console_.RegisterCommand("stop", [this](char *){
            Log(P("Stop"));

            MsgTxnStop msg;

            SendAndApply(msg);
        });

        console_.RegisterCommand("status", [this](char *){
            MsgTxnDumpStatus msg;

            SendAndApply(msg);
        });

        console_.SetVerbose(0);
        console_.Start();
    }

private:

    SerialAsyncConsoleEnhanced<10>  console_;

    AppBikeLightComputerRemoteConfig &cfg_;
};

























#endif  // __APP_BIKE_LIGHT_COMPUTER_H__