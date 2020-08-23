#ifndef __APP_BIKE_LIGHT_COMPUTER_H__
#define __APP_BIKE_LIGHT_COMPUTER_H__


#include "Log.h"
#include "Evm.h"
#include "TimedEventHandler.h"
#include "SerialInput.h"
#include "PinInput.h"
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

struct MsgTxnPct
{
    static const uint8_t MSG_TYPE = 5;
    uint8_t msgType = MSG_TYPE;

    uint8_t pct = 100;
};

struct MsgTxnMode
{
    static const uint8_t MSG_TYPE = 6;
    uint8_t msgType = MSG_TYPE;

    enum Color : uint8_t
    {
        RED,
        GREEN,
        BLUE,
    };

    Color color;
    RgbLedEffectsController::ChannelMode mode;
    uint8_t val;
};

struct MsgTxnMultiMode
{
    static const uint8_t MSG_TYPE = 7;
    uint8_t msgType = MSG_TYPE;

    MsgTxnMode red;
    MsgTxnMode green;
    MsgTxnMode blue;
};



// Compile-time warn about sizing
static_assert(sizeof(MsgTxnApply) <= RFLink::MAX_PACKET_SIZE, "MsgTxnApply too large");







struct AppBikeLightComputerConfig
{
    // interfacing config
    uint8_t pinConfigure;
    uint8_t pinInputStartStop;

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

    static const uint8_t DEFAULT_PCT_RANGE = 30;


public:
    AppBikeLightComputer(AppBikeLightComputerConfig &cfg)
    : cfg_(cfg)
    , radio_(cfg_.pinIrq, cfg_.pinSdn, cfg_.pinSel)
    {
        // Nothing to do
    }

    void Run()
    {
        Init();
        RunInternal();
    }


protected:

    void Init()
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

            rgb_.SetRangePct(DEFAULT_PCT_RANGE);
        }
        else
        {
            Log(P("Bad Configuration"));
            while (1) {}
        }
    }

    void RunInternal()
    {
        SetUpEffects();
        SetUpRadio();

        evm_.MainLoop();
    }

    void OnMsg(const MsgTxnDumpStatus &)
    {
        radio_.DumpStatus();
    }

    void OnMsg(const MsgTxnApply &msg)
    {
        Log("Got TxnApply: ", msg.msgType);

        // 625us at 8MHz
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

    void OnMsg(const MsgTxnPct &msg)
    {
        Log("Got TxnPct: ", msg.msgType, ", ", msg.pct);

        rgb_.SetRangePct(msg.pct);
    }

    void OnMsg(const MsgTxnMode &msg)
    {
        // Eliminate logging on high-throughput messages
        // Log("Got TxnMode");

        if (msg.color == MsgTxnMode::Color::RED)
        {
            rgb_.SetModeRed(msg.mode);
            if (msg.mode == RgbLedEffectsController::ChannelMode::LITERAL)
            {
                rgb_.SetLiteralRed(msg.val);
            }
        }
        else if (msg.color == MsgTxnMode::Color::GREEN)
        {
            rgb_.SetModeGreen(msg.mode);
            if (msg.mode == RgbLedEffectsController::ChannelMode::LITERAL)
            {
                rgb_.SetLiteralGreen(msg.val);
            }
        }
        else if (msg.color == MsgTxnMode::Color::BLUE)
        {
            rgb_.SetModeBlue(msg.mode);
            if (msg.mode == RgbLedEffectsController::ChannelMode::LITERAL)
            {
                rgb_.SetLiteralBlue(msg.val);
            }
        }
    }

    void OnMsg(const MsgTxnMultiMode &msg)
    {
        // Eliminate logging on high-throughput messages
        // Log("Got TxnMultiMode");

        OnMsg(msg.red);
        OnMsg(msg.green);
        OnMsg(msg.blue);
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
            CheckAndDispatch<MsgTxnPct>(buf, bufLen);
            CheckAndDispatch<MsgTxnMode>(buf, bufLen);
            CheckAndDispatch<MsgTxnMultiMode>(buf, bufLen);
        });

        radio_.ModeReceive();
    }


private:

    static const uint8_t C_IDLE  =  0;
    static const uint8_t C_TIMED =  5;
    static const uint8_t C_INTER =  1;
    
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
    , piStartStop_(cfg_.cfg.pinInputStartStop)
    {
        // Nothing to do
    }

    void Run()
    {
        // Run base class Init
        AppBikeLightComputer::Init();

        // Add additional behavior
        SetUpRadio();
        SetUpCommandHandler();
        SetUpInitialState();
        InputsEnable();

        // Run base class
        AppBikeLightComputer::RunInternal();
    }

private:

    void SetUpRadio()
    {
        radio_.SetOnMessageTransmittedCallback([this](){
            // Eliminate logging on high-throughput messages
            //Log(P("Msg Sent"));
            InputsEnable();
        });
    }

    void InputsEnable()
    {
        static uint8_t onOff = 0;

        piStartStop_.SetCallback([this](uint8_t){
            if (onOff)
            {
                SendAndApplyTxnStart();
            }
            else
            {
                SendAndApplyTxnStop();
            }

            onOff = !onOff;
        });
        piStartStop_.Enable();
    }

    void InputsDisable()
    {

    }

    template <typename T>
    void Send(const T &t)
    {
        // Send to remote systems
        radio_.Send((uint8_t *)&t, sizeof(t));
    }

    template <typename T>
    void SendAndApply(const T &t)
    {
        // Send to remote systems
        Send(t);

        // Wait for transmission to complete before allowing new ones
        InputsDisable();

        // Apply internally
        OnMsg(t);
    }

    void SendAndApplyTxnStart()
    {
        MsgTxnStart msg;

        SendAndApply(msg);
    }

    void SendAndApplyTxnStop()
    {
        MsgTxnStop msg;

        SendAndApply(msg);
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
        console_.RegisterErrorHandler([this](char *cmdStr){
            Str str(cmdStr);
            const char *cmd = str.TokenAtIdx(0, ' ');
            

            ////////////////////////////////////////////////////////////////////
            //
            // Commands that are applied both to remote and bike computers.
            // These states are meant to be kept in sync.
            //
            ////////////////////////////////////////////////////////////////////

            if (!strcmp_P(cmd, P("start")))
            {
                Log(P("Start"));

                SendAndApplyTxnStart();
            }
            else if (!strcmp_P(cmd, P("mro")))
            {
                MsgTxnMode msg;
                msg.color = MsgTxnMode::Color::RED;
                msg.mode = RgbLedEffectsController::ChannelMode::OSCILLATOR;

                Log(P("MRO"));

                SendAndApply(msg);
            }
            else if (!strcmp_P(cmd, P("mgo")))
            {
                MsgTxnMode msg;
                msg.color = MsgTxnMode::Color::GREEN;
                msg.mode = RgbLedEffectsController::ChannelMode::OSCILLATOR;

                Log(P("MGO"));

                SendAndApply(msg);
            }
            else if (!strcmp_P(cmd, P("mbo")))
            {
                MsgTxnMode msg;
                msg.color = MsgTxnMode::Color::BLUE;
                msg.mode = RgbLedEffectsController::ChannelMode::OSCILLATOR;

                Log(P("MBO"));

                SendAndApply(msg);
            }
            else if (!strcmp_P(cmd, P("mrl")))
            {
                MsgTxnMode msg;
                msg.color = MsgTxnMode::Color::RED;
                msg.mode = RgbLedEffectsController::ChannelMode::LITERAL;

                if (str.TokenCount(' ') == 2)
                {
                    uint8_t val = atoi(str.TokenAtIdx(1, ' '));

                    msg.val = val;

                    // Log(P("MRL "), msg.val);

                    SendAndApply(msg);
                }
            }
            else if (!strcmp_P(cmd, P("mgl")))
            {
                MsgTxnMode msg;
                msg.color = MsgTxnMode::Color::GREEN;
                msg.mode = RgbLedEffectsController::ChannelMode::LITERAL;

                if (str.TokenCount(' ') == 2)
                {
                    uint8_t val = atoi(str.TokenAtIdx(1, ' '));

                    msg.val = val;

                    // Log(P("MGL "), msg.val);

                    SendAndApply(msg);
                }
            }
            else if (!strcmp_P(cmd, P("mbl")))
            {
                MsgTxnMode msg;
                msg.color = MsgTxnMode::Color::BLUE;
                msg.mode = RgbLedEffectsController::ChannelMode::LITERAL;

                if (str.TokenCount(' ') == 2)
                {
                    uint8_t val = atoi(str.TokenAtIdx(1, ' '));

                    msg.val = val;

                    // Log(P("MBL "), msg.val);

                    SendAndApply(msg);
                }
            }
            else if (!strcmp_P(cmd, P("rgb")))
            {
                MsgTxnMultiMode msg;

                if (str.TokenCount(' ') == 4)
                {
                    uint8_t valR = atoi(str.TokenAtIdx(1, ' '));
                    uint8_t valG = atoi(str.TokenAtIdx(2, ' '));
                    uint8_t valB = atoi(str.TokenAtIdx(3, ' '));

                    msg.red.color = MsgTxnMode::Color::RED;
                    msg.red.mode  = RgbLedEffectsController::ChannelMode::LITERAL;
                    msg.red.val   = valR;

                    msg.green.color = MsgTxnMode::Color::GREEN;
                    msg.green.mode  = RgbLedEffectsController::ChannelMode::LITERAL;
                    msg.green.val   = valG;

                    msg.blue.color = MsgTxnMode::Color::BLUE;
                    msg.blue.mode  = RgbLedEffectsController::ChannelMode::LITERAL;
                    msg.blue.val   = valB;

                    // Log(P("RGB "), msg.red.val, " ", msg.green.val, " ", msg.blue.val);

                    SendAndApply(msg);
                }
            }
            else if (!strcmp_P(cmd, P("apply")))
            {
                MsgTxnApply msg;

                if (str.TokenCount(' ') == 2)
                {
                    uint32_t val = atoi(str.TokenAtIdx(1, ' '));

                    msg.alsoStart = val;
                }

                Log(P("Apply "), msg.alsoStart ? P("and Start") : P("but not Start"));

                msg.rgbColorState = rgb_.GetState();

                SendAndApply(msg);
            }
            else if (!strcmp_P(cmd, P("aas")))
            {
                Log(P("ApplyAndStart"));

                console_.Exec("apply 1");
            }
            else if (!strcmp_P(cmd, P("pause")))
            {
                Log(P("Pause"));

                MsgTxnPause msg;

                SendAndApply(msg);
            }
            else if (!strcmp_P(cmd, P("stop")))
            {
                Log(P("Stop"));

                SendAndApplyTxnStop();
            }


            ////////////////////////////////////////////////////////////////////
            //
            // Commands that are forwarded to the bike computers but don't
            // affect the state of the remote
            //
            ////////////////////////////////////////////////////////////////////

            else if (!strcmp_P(cmd, P("pct")))
            {
                MsgTxnPct msg;

                if (str.TokenCount(' ') == 2)
                {
                    uint8_t val = atoi(str.TokenAtIdx(1, ' '));

                    // Log(P("Pct "), val);

                    msg.pct = val;

                    Send(msg);
                }
            }


            ////////////////////////////////////////////////////////////////////
            //
            // Commands that apply to the remote itself
            //
            ////////////////////////////////////////////////////////////////////

            else if (!strcmp_P(cmd, P("sync")))
            {
                if (str.TokenCount(' ') == 2)
                {
                    uint8_t val = atoi(str.TokenAtIdx(1, ' '));

                    Log(P("Sync "), val);

                    if (val)
                    {
                        ted_.SetCallback([this](){
                            console_.Exec("apply");
                        });
                        ted_.RegisterForTimedEventInterval(val * 1000, 0);
                    }
                    else
                    {
                        ted_.DeRegisterForTimedEvent();
                    }
                }
            }
            else if (!strcmp_P(cmd, P("power")))
            {
                MsgTxnApply msg;

                if (str.TokenCount(' ') == 2)
                {
                    uint32_t val = atoi(str.TokenAtIdx(1, ' '));

                    Log(P("Power "), val);

                    radio_.SetTxPower(val);

                    msg.alsoStart = val;
                }
            }


            ////////////////////////////////////////////////////////////////////
            //
            // Debug
            //
            ////////////////////////////////////////////////////////////////////

            else if (!strcmp_P(cmd, P("status")))
            {
                MsgTxnDumpStatus msg;

                SendAndApply(msg);
            }
        });

        console_.SetVerbose(0);
        console_.Start();
    }

    void SetUpInitialState()
    {
        console_.Exec("sync 5");
    }

private:

    SerialAsyncConsoleEnhanced<0>  console_;

    AppBikeLightComputerRemoteConfig &cfg_;

    PinInput piStartStop_;

    TimedEventHandlerDelegate ted_;
};

























#endif  // __APP_BIKE_LIGHT_COMPUTER_H__