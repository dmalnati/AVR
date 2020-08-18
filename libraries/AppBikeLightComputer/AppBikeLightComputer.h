#ifndef __APP_BIKE_LIGHT_COMPUTER_H__
#define __APP_BIKE_LIGHT_COMPUTER_H__


#include "Log.h"
#include "Evm.h"
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


    struct RuntimeConfiguration
    {
        // This will only be changed on the remotely-controlled.
        // The master state keeper, the remote, will never scale down.
        uint8_t pctRange = 100;
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

            // Pull in stored configuration if any
            ReadRuntimeConfig();
            Log(P("Read Runtime Config"));
            Log(P("  Pct: "), runtimeCfg_.pctRange);
            rgb_.SetRangePct(runtimeCfg_.pctRange);
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

        runtimeCfg_.pctRange = msg.pct;
        WriteRuntimeConfig();
        rgb_.SetRangePct(runtimeCfg_.pctRange);
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
        });

        radio_.ModeReceive();
    }

    void ReadRuntimeConfig()
    {
        ea_.Read(runtimeCfg_);
    }

    void WriteRuntimeConfig()
    {
        ea_.Write(runtimeCfg_);
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

    RuntimeConfiguration                 runtimeCfg_;
    EepromAccessor<RuntimeConfiguration> ea_;
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
        InputsEnable();

        // Run base class
        AppBikeLightComputer::RunInternal();
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
        static uint8_t onOff = 0;

        piStartStop_.SetCallback([this](uint8_t){
            if (onOff)
            {
                SendTxnStart();
            }
            else
            {
                SendTxnStop();
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

    void SendTxnStart()
    {
        MsgTxnStart msg;

        SendAndApply(msg);
    }

    void SendTxnStop()
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
            
            if (!strcmp_P(cmd, P("apply")))
            {
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
            }
            else if (!strcmp_P(cmd, P("start")))
            {
                Log(P("Start"));

                SendTxnStart();
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

                SendTxnStop();
            }
            else if (!strcmp_P(cmd, P("pct")))
            {
                MsgTxnPct msg;

                if (str.TokenCount(' ') == 2)
                {
                    uint8_t val = atoi(str.TokenAtIdx(1, ' '));

                    Log(P("Pct "), val);

                    msg.pct = val;

                    Send(msg);
                }
            }
            else if (!strcmp_P(cmd, P("status")))
            {
                MsgTxnDumpStatus msg;

                SendAndApply(msg);
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
        });

        console_.SetVerbose(0);
        console_.Start();
    }

private:

    SerialAsyncConsoleEnhanced<0>  console_;

    AppBikeLightComputerRemoteConfig &cfg_;

    PinInput piStartStop_;
};

























#endif  // __APP_BIKE_LIGHT_COMPUTER_H__