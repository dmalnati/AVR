#ifndef __SERIAL_INPUT_H__
#define __SERIAL_INPUT_H__


#include "Evm.h"
#include "Serial.h"
#include "Log.h"
#include "Function.h"
#include "Str.h"
#include "StreamWindow.h"
#include "TimedEventHandler.h"




class SerialAsyncReadLine
{
    static const uint32_t DEFAULT_POLL_PERIOD_MS = 25;
    static const uint8_t  MAX_READS_PER_POLL     = 80;
    
public:
    SerialAsyncReadLine()
    : SerialAsyncReadLine(NULL, 0)
    {
        // Nothing to do
    }
    
    SerialAsyncReadLine(char *buf, uint8_t bufSize)
    : pollPeriodMs_(DEFAULT_POLL_PERIOD_MS)
    , running_(0)
    {
        Attach(buf, bufSize);
    }
    
    uint8_t Attach(char *buf, uint8_t bufSize)
    {
        Stop();
        
        uint8_t retVal = 0;
        
        buf_ = NULL;
        bufSize_ = 0;
        
        if (buf && bufSize)
        {
            retVal = 1;
            
            buf_     = buf;
            bufSize_ = bufSize;
            
            // Trick the window into thinking our buffer is one element
            // shorter than it is, to help us maintain a c-string.
            // Then terminate the string with a NULL which will never be
            // overwritten.
            sw_.Attach(buf_, bufSize_ - 1, '\0');
            buf_[bufSize_ - 1] = '\0';
        }
        
        return retVal;
    }
    
    void SetCallback(function<void(char *)> cbFn)
    {
        cbFn_ = cbFn;
    }
    
    function<void(char *)> GetCallback()
    {
        return cbFn_;
    }
    
    void SetPollPeriod(uint32_t pollPeriodMs)
    {
        pollPeriodMs_ = pollPeriodMs;
        
        if (running_)
        {
            Start();
        }
    }
    
    uint8_t Start()
    {
        uint8_t retVal = 0;
        
        Stop();
        
        if (buf_)
        {
            ted_.SetCallback([this](){ OnPoll(); });
            
            retVal = ted_.RegisterForTimedEventInterval(pollPeriodMs_);
            
            running_ = !!retVal;
        }
        
        return retVal;
    }
    
    uint8_t Stop()
    {
        uint8_t retVal = 0;
        
        if (buf_)
        {
            retVal = ted_.DeRegisterForTimedEvent();
        }
        
        running_ = 0;
        
        return retVal;
    }
    

private:

    void OnPoll()
    {
        uint8_t readsRemaining = MAX_READS_PER_POLL;
        
        uint8_t cont = 1;
        
        while (cont)
        {
            if (S0.Available())
            {
                char c = S0.Read();
                
                if (c == '\n')
                {
                    cbFn_(buf_);
                    
                    sw_.Reset();
                }
                else
                {
                    // Check if there is room to fit one more
                    if (sw_.CanFitOneMore())
                    {
                        sw_.Append(c);
                    }
                    else
                    {
                        // throw away the character, we only want the start of lines
                    }
                }
                
                --readsRemaining;
                if (!readsRemaining)
                {
                    cont = 0;
                }
            }
            else
            {
                cont = 0;
            }
        }
    }


    char                   *buf_;
    uint8_t                 bufSize_;
    function<void(char *)>  cbFn_;
    
    uint32_t pollPeriodMs_;
    
    TimedEventHandlerDelegate ted_;
    
    uint8_t running_;
    
    StreamWindow<char> sw_;
};


template <uint8_t NUM_COMMANDS, uint8_t NUM_BYTES_SERIAL = 30>
class SerialAsyncConsole
{
    static const uint8_t BUF_SIZE = NUM_BYTES_SERIAL + 1;
    
    struct CmdToFn
    {
        const char                   *cmd;
        int8_t                        reqArgCount;
        function<void(char *cmdStr)>  fn;
    };
    
public:
    SerialAsyncConsole()
    : cmdToFnListIdx_(0)
    , verbose_(1)
    , isRunning_(0)
    {
        // Nothing to do
    }
    
    uint8_t RegisterCommand(const char                   *cmd,
                            uint8_t                       reqArgCount,
                            function<void(char *cmdStr)>  fn)
    {
        uint8_t retVal = 0;
        
        if (cmdToFnListIdx_ < NUM_COMMANDS)
        {
            retVal = 1;
            
            CmdToFn ctf{ cmd, (int8_t)reqArgCount, fn };
            
            cmdToFnList_[cmdToFnListIdx_] = ctf;
            
            ++cmdToFnListIdx_;
        }
        
        return retVal;
    }
    
    uint8_t RegisterCommand(const char                   *cmd,
                            function<void(char *cmdStr)>  fn)
    {
        return RegisterCommand(cmd, (uint8_t)-1, fn);
    }
    
    void RegisterErrorHandler(function<void(char *cmdStr)> fnErr)
    {
        fnErr_ = fnErr;
    }
    
    void SetVerbose(uint8_t verbose)
    {
        verbose_ = verbose;
    }
    
    void Start()
    {
        isRunning_ = 1;
        
        sarl_.Attach(buf_, BUF_SIZE);
        sarl_.SetCallback([this](char *strCmd){
            uint8_t strLen = strlen(strCmd);
            
            if (strLen)
            {
                Str str(strCmd);
                
                const char *cmd = str.TokenAtIdx(0, ' ');

                uint8_t found = 0;
                for (uint8_t i = 0; i < cmdToFnListIdx_ && !found; ++i)
                {
                    if (!strcmp(cmd, cmdToFnList_[i].cmd))
                    {
                        found = 1;
                        
                        // Check if min arguments apply, and if yes, if present
                        uint8_t executeFunction = 1;
                        if (cmdToFnList_[i].reqArgCount != -1)
                        {
                            uint8_t argCount = str.TokenCount(' ') - 1;
                            
                            if (argCount != cmdToFnList_[i].reqArgCount)
                            {
                                executeFunction = 0;
                                
                                if (verbose_)
                                {
                                    Log(P("ERR: "),
                                        cmdToFnList_[i].cmd,
                                        ' ',
                                        argCount,
                                        '/',
                                        cmdToFnList_[i].reqArgCount,
                                        P(" args provided"));
                                }
                            }
                        }
                        
                        str.Release();
                        
                        if (executeFunction)
                        {
                            cmdToFnList_[i].fn(strCmd);
                        }
                    }
                }
                
                if (!found)
                {
                    str.Release();
                        
                    fnErr_(strCmd);
                }
                
                if (verbose_)
                {
                    LogNL();
                }
            }            
        });
        sarl_.Start();
        
        if (verbose_)
        {
            Log(P("Commands:"));
            for (uint8_t i = 0; i < cmdToFnListIdx_; ++i)
            {
                if (cmdToFnList_[i].reqArgCount == -1)
                {
                    Log(cmdToFnList_[i].cmd);
                }
                else
                {
                    Log(cmdToFnList_[i].cmd, '<', cmdToFnList_[i].reqArgCount, '>');
                }                
            }
            LogNL();
        }
    };
    
    void Stop()
    {
        isRunning_ = 0;
        
        sarl_.Stop();
    }
    
    uint8_t IsRunning()
    {
        return isRunning_;
    }
    
    void Exec(const char *strCmd)
    {
        sarl_.GetCallback()((char *)strCmd);
    }

private:
    char buf_[BUF_SIZE];
    
    CmdToFn cmdToFnList_[NUM_COMMANDS];
    uint8_t cmdToFnListIdx_;
    
    function<void(char *cmdStr)> fnErr_;

    SerialAsyncReadLine sarl_;
    
    uint8_t verbose_;
    
    uint8_t isRunning_;
};



template <uint8_t NUM_COMMANDS, uint8_t NUM_BYTES_SERIAL = 30>
class SerialAsyncConsoleEnhanced
: public SerialAsyncConsole<NUM_COMMANDS + 3, NUM_BYTES_SERIAL>
{
public:

    SerialAsyncConsoleEnhanced(uint8_t enableDefaultErrorHandler = 1)
    {
        Init(enableDefaultErrorHandler);
    }

    
private:

    void Init(uint8_t enableDefaultErrorHandler)
    {
        // sub-commands:
        // - pin set <pin> <val>
        // - pin get <pin> <doPullup>
        // - pin aget <pin> <aref/1.1> // analog
        this->RegisterCommand("pin", 3, [](char *cmdStr) {
            Str str(cmdStr);
            
            const char *getSetStr =        str.TokenAtIdx(1, ' ');
            uint8_t     pin       =   atoi(str.TokenAtIdx(2, ' '));
            uint8_t     val       = !!atoi(str.TokenAtIdx(3, ' '));
            const char *p3       =         str.TokenAtIdx(3, ' ');
            
            if (!strcmp_P(getSetStr, P("set")))
            {
                PAL.PinMode(pin, OUTPUT);
                
                Log(P("Pin "), pin, P(" -> "), val);

                PAL.DigitalWrite(pin, val);
            }
            else if (!strcmp_P(getSetStr, P("get")))
            {
                PAL.PinMode(pin, val ? INPUT_PULLUP : INPUT);
                
                uint8_t valRead = PAL.DigitalRead(pin);

                Log(P("Pin "), pin, P("("), val, P(") <- "), valRead);
            }
            else if (!strcmp_P(getSetStr, P("aget")))
            {
                uint16_t valRead = 0;
                
                if (!strcmp_P(p3, P("aref")))
                {
                    valRead = PAL.AnalogRead(pin);
                }
                else if (!strcmp_P(p3, P("1.1")))
                {
                    valRead = PAL.AnalogRead1V1(pin);
                }

                Log(P("Pin "), pin, P("("), val, P(") <- "), valRead);
            }
        });
        
        this->RegisterCommand("reset", [](char *){
            PAL.SoftReset();
        });
        
        this->RegisterCommand("dlp", 1, [](char *cmdStr){
            Str str(cmdStr);

            uint32_t delayMs = atol(str.TokenAtIdx(1, ' '));
            
            Log(P("DelayLowPower "), delayMs, P(" ms"));
            
            PAL.DelayLowPower(delayMs);
            
            Log(P("  awake again"));
        });
        
        if (enableDefaultErrorHandler)
        {
            this->RegisterErrorHandler([](char *cmdStr) {
                Log(P("ERR: \""), cmdStr, '\"');
            });
        }
    }
};



class SerialReadLineClass
{
public:

    static uint8_t SerialReadLine(char *buf, uint8_t bufSize)
    {
        uint8_t retVal = 0;
     
        while (!retVal)
        {
            
            retVal = S0.ReadBytesUntil('\n', (uint8_t *)buf, bufSize);

            if (retVal < bufSize)
            {
                buf[retVal] = '\0';
            }
            else
            {
                buf[bufSize - 1] = '\0';
                
                // newline wasn't hit, and bytes weren't discarded, so consume them
                uint8_t cont = 1;
                while (cont)
                {
                    if (S0.Available())
                    {
                        char c = S0.Read();
                        
                        if (c == '\n')
                        {
                            cont = 0;
                        }
                    }
                }
            }
        }
        
        return retVal;
    }
};

template <uint8_t NUM_COMMANDS, uint8_t NUM_BYTES_SERIAL = 100>
class SerialShell
{
    static const uint8_t BUF_SIZE = NUM_BYTES_SERIAL + 1;
    
    struct CmdToFn
    {
        const char                   *cmd;
        function<void(char *cmdStr)>  fn;
    };
    
public:
    SerialShell()
    : cmdToFnListIdx_(0)
    , running_(1)
    {
        // Nothing to do
    }
    
    uint8_t RegisterCommand(const char                    *cmd,
                            function<void(char *cmdStr)>  fn)
    {
        uint8_t retVal = 0;
        
        if (cmdToFnListIdx_ < NUM_COMMANDS)
        {
            retVal = 1;
            
            CmdToFn ctf{ cmd, fn };
            
            cmdToFnList_[cmdToFnListIdx_] = ctf;
            
            ++cmdToFnListIdx_;
        }
        
        return retVal;
    }
    
    void RegisterErrorHandler(function<void(char *cmdStr)> fnErr)
    {
        fnErr_ = fnErr;
    }
    
    void Run()
    {
        Log(P("Commands:"));
        for (uint8_t i = 0; i < cmdToFnListIdx_; ++i)
        {
            Log(cmdToFnList_[i].cmd);
        }
        LogNL();

        while (running_)
        {
            uint8_t strLen = SerialReadLineClass::SerialReadLine(buf_, BUF_SIZE);

            if (strLen)
            {
                Str str(buf_);
                
                const char *cmd = str.TokenAtIdx(0, ' ');

                uint8_t found = 0;
                for (uint8_t i = 0; i < cmdToFnListIdx_ && !found; ++i)
                {
                    if (!strcmp(cmd, cmdToFnList_[i].cmd))
                    {
                        found = 1;
                        
                        str.Release();
                        
                        cmdToFnList_[i].fn(buf_);
                    }
                }
                
                if (!found)
                {
                    str.Release();
                        
                    fnErr_(buf_);
                }
                
                LogNL();
            }
        }
    };

private:
    char buf_[BUF_SIZE];
    
    CmdToFn cmdToFnList_[NUM_COMMANDS];
    uint8_t cmdToFnListIdx_;
    
    function<void(char *cmdStr)> fnErr_;

    uint8_t running_;
};




template <uint8_t PARAM_COUNT, uint8_t CMD_COUNT, uint8_t FORMATTER_COUNT = 0, uint8_t MAX_LINE_LEN = 40>
class SerialAsyncConsoleMenu
{
public:

    enum ParamType
    {
        STR,
        U32,
        U16,
        U8,
        I32,
        I16,
        I8,
    };
    
    struct Param
    {
        Param()
        {
        }
        
        Param(PStr       paramNameInput,
              ParamType  paramTypeInput,
              void      *paramPtrInput,
              uint8_t    paramDataInput,
              int8_t     idxFormatterInput = -1)
        : paramName(paramNameInput)
        , paramType(paramTypeInput)
        , paramPtr(paramPtrInput)
        , paramData(paramDataInput)
        , idxFormatter(idxFormatterInput)
        {
            // Nothing to do
        }
        
        PStr       paramName;
        ParamType  paramType;
        void      *paramPtr;
        uint8_t    paramData;
        int8_t     idxFormatter;
    };
    
    
private:
    struct Command
    {
        Command()
        {
        }
        
        Command(PStr cmdInput, function<void(void)> &fnCmdInput)
        : cmd(cmdInput)
        , fnCmd(fnCmdInput)
        {
            // Nothing to do
        }
        
        PStr                  cmd;
        function<void(void)>  fnCmd;
    };
    
    
public:

    int8_t RegisterFormatter(function<void(Param &param)> fnFormatter)
    {
        int8_t retVal = -1;
        
        if (fnFormatterListIdx_ < FORMATTER_COUNT)
        {
            retVal = fnFormatterListIdx_;
            
            fnFormatterList_[fnFormatterListIdx_] = fnFormatter;
            ++fnFormatterListIdx_;
        }
        
        return retVal;
    }
    
    // assumes there's a null beyond the strLen bytes
    uint8_t RegisterParamSTR(PStr paramName, const char *paramPtr, uint8_t strLen, int8_t fnFormatterIdx = -1)
    {
        return RegisterParam({ paramName, ParamType::STR, (void *)paramPtr, strLen, fnFormatterIdx });
    }
    
    uint8_t RegisterParamU32(PStr paramName, uint32_t *paramPtr, int8_t fnFormatterIdx = -1)
    {
        return RegisterParam({ paramName, ParamType::U32, (void *)paramPtr, 0, fnFormatterIdx });
    }
    
    uint8_t RegisterParamU16(PStr paramName, uint16_t *paramPtr, int8_t fnFormatterIdx = -1)
    {
        return RegisterParam({ paramName, ParamType::U16, (void *)paramPtr, 0, fnFormatterIdx });
    }
    
    uint8_t RegisterParamU8(PStr paramName, uint8_t *paramPtr, int8_t fnFormatterIdx = -1)
    {
        return RegisterParam({ paramName, ParamType::U8, (void *)paramPtr, 0, fnFormatterIdx });
    }

    uint8_t RegisterParamI32(PStr paramName, int32_t *paramPtr, int8_t fnFormatterIdx = -1)
    {
        return RegisterParam({ paramName, ParamType::I32, (void *)paramPtr, 0, fnFormatterIdx });
    }
    
    uint8_t RegisterParamI16(PStr paramName, int16_t *paramPtr, int8_t fnFormatterIdx = -1)
    {
        return RegisterParam({ paramName, ParamType::I16, (void *)paramPtr, 0, fnFormatterIdx });
    }
    
    uint8_t RegisterParamI8(PStr paramName, int8_t *paramPtr, int8_t fnFormatterIdx = -1)
    {
        return RegisterParam({ paramName, ParamType::I8, (void *)paramPtr, 0, fnFormatterIdx });
    }

    void SetOnSetCallback(function<void(void)> cbFnOnSet)
    {
        cbFnOnSet_ = cbFnOnSet;
    }
    
    uint8_t RegisterCommand(PStr cmd, function<void(void)> fnCmd)
    {
        uint8_t retVal = 0;
        
        if (commandListIdx_ < CMD_COUNT)
        {
            retVal = 1;
            
            commandList_[commandListIdx_] = { cmd, fnCmd };
            ++commandListIdx_;
        }
        
        return retVal;
    }
    
    void Start()
    {
        // We're using the error handler as a catch-all, we'll dissect the
        // typed string ourselves.
        console_.RegisterErrorHandler([this](char *cmdStr){
            OnInput(cmdStr);
        });
        
        console_.SetVerbose(0);
        console_.Start();
    }
    
    void ShowParams()
    {
        // determine the max len of any param
        uint8_t maxLen = 0;
        for (uint8_t i = 0; i < paramListIdx_; ++i)
        {
            Param &param = paramList_[i];
            
            uint8_t len = strlen_P(param.paramName);
            
            if (param.paramType == ParamType::STR)
            {
                len += 2;                             // for the square brackets
                len += param.paramData < 10 ? 1 : 2;  // for the char count
            }
            
            if (len > maxLen)
            {
                maxLen = len;
            }
        }
        
        Log(P("Parameters: (set <param> <val>)"));
        for (uint8_t i = 0; i < paramListIdx_; ++i)
        {
            Param &param = paramList_[i];
            
            uint8_t len = strlen_P(param.paramName);
            
            LogNNL(param.paramName);
            if (param.paramType == ParamType::STR)
            {
                len += 2;                             // for the square brackets
                len += param.paramData < 10 ? 1 : 2;  // for the char count
                
                LogNNL('[', param.paramData, "]");
            }

            uint8_t pad = maxLen - len;
            LogXNNL(' ', pad);
            
            LogNNL(P(" : "));
            
                 if (param.paramType == ParamType::STR) { LogParamSTR(param); }
            else if (param.paramType == ParamType::U32) { LogParamU32(param); }
            else if (param.paramType == ParamType::U16) { LogParamU16(param); }
            else if (param.paramType == ParamType::U8)  { LogParamU8(param);  }
            else if (param.paramType == ParamType::I32) { LogParamI32(param); }
            else if (param.paramType == ParamType::I16) { LogParamI16(param); }
            else if (param.paramType == ParamType::I8)  { LogParamI8(param);  }
            
            if (param.idxFormatter >= 0 && param.idxFormatter < fnFormatterListIdx_)
            {
                LogNNL(P(" \t"));
                fnFormatterList_[param.idxFormatter](param);
            }
            
            LogNL();
        }
    }
    
    void ShowCommands()
    {
        Log(P("Commands:"));
        for (uint8_t i = 0; i < commandListIdx_; ++i)
        {
            Log(commandList_[i].cmd);
        }
    }
    
    void ShowAll()
    {
        ShowCommands();
        LogNL();
        ShowParams();
        LogNL(2);
    }
    
    
private:

    uint8_t RegisterParam(Param param)
    {
        uint8_t retVal = 0;
        if (paramListIdx_ < PARAM_COUNT)
        {
            retVal = 1;
            
            paramList_[paramListIdx_] = param;
            ++paramListIdx_;
        }
        
        return retVal;
    }
    
    void OnInput(char *cmdStr)
    {
        Str str(cmdStr);
        
        uint8_t     tokenCount = str.TokenCount(' ');
        const char *command    = str.TokenAtIdx(0, ' ');
        const char *name       = str.TokenAtIdx(1, ' ');
        
        uint8_t commandNotFound = 0;
        
        if (tokenCount == 3)
        {
            if (!strcmp(command, "set"))
            {
                const char *value = str.TokenAtIdx(2, ' ');
                
                uint8_t found = 0;
                for (uint8_t i = 0; i < paramListIdx_ && !found; ++i)
                {
                    Param &param = paramList_[i];

                    if (!strcmp_P(name, param.paramName))
                    {
                        found = 1;
                        
                        Log(P("Setting "), name, P(" to "), value);
                        
                             if (param.paramType == ParamType::STR) { SetParamSTR(param, value); }
                        else if (param.paramType == ParamType::U32) { SetParamU32(param, value); }
                        else if (param.paramType == ParamType::U16) { SetParamU16(param, value); }
                        else if (param.paramType == ParamType::U8)  { SetParamU8(param, value);  }
                        else if (param.paramType == ParamType::I32) { SetParamI32(param, value); }
                        else if (param.paramType == ParamType::I16) { SetParamI16(param, value); }
                        else if (param.paramType == ParamType::I8)  { SetParamI8(param, value);  }
                        
                        cbFnOnSet_();
                    }
                }
                
                if (!found)
                {
                    Log(P("ERR: <param> "), name, P(" not found"));
                }
            }
            else
            {
                commandNotFound = 1;
            }
        }
        else if (tokenCount == 1)
        {
            uint8_t found = 0;
            for (uint8_t i = 0; i < commandListIdx_ && !found; ++i)
            {
                Command &cmd = commandList_[i];

                if (!strcmp_P(command, cmd.cmd))
                {
                    found = 1;
                    
                    cmd.fnCmd();
                }
            }
            
            if (!found)
            {
                if (!strcmp(command, "?") || !strcmp(command, "help"))
                {
                    found = 1;
                    
                    LogNL();
                    ShowAll();
                }
            }
            
            commandNotFound = !found;
        }
        else
        {
            commandNotFound = 1;
        }
        
        if (commandNotFound)
        {
            Log(P("ERR: <command> "), command, P(" not found"));
        }
        
        LogNL();
    }
    
    void SetParamSTR(Param &param, const char *value)
    {
        const char *paramBuf = (const char *)param.paramPtr;
        uint8_t strLen       = param.paramData;
        
        memset((void *)paramBuf, '\0', strLen);
        strncpy((char *)paramBuf, value, strLen);
    }
    
    void SetParamU32(Param &param, const char *value)
    {
        *(uint32_t *)param.paramPtr = atol(value);
    }
    void SetParamU16(Param &param, const char *value)
    {
        *(uint16_t *)param.paramPtr = atol(value);
    }
    void SetParamU8(Param &param, const char *value)
    {
        *(uint8_t *)param.paramPtr = atol(value);
    }
    
    void SetParamI32(Param &param, const char *value)
    {
        *(int32_t *)param.paramPtr = atol(value);
    }
    void SetParamI16(Param &param, const char *value)
    {
        *(int16_t *)param.paramPtr = atol(value);
    }
    void SetParamI8(Param &param, const char *value)
    {
        *(int8_t *)param.paramPtr = atoi(value);
    }
    
    void LogParamSTR(Param &param)
    {
        LogNNL((const char *)param.paramPtr);
    }
    void LogParamU32(Param &param)
    {
        LogNNL(*(uint32_t *)param.paramPtr);
    }
    void LogParamU16(Param &param)
    {
        LogNNL(*(uint16_t *)param.paramPtr);
    }
    void LogParamU8(Param &param)
    {
        LogNNL(*(uint8_t *)param.paramPtr);
    }
    void LogParamI32(Param &param)
    {
        LogNNL(*(int32_t *)param.paramPtr);
    }
    void LogParamI16(Param &param)
    {
        LogNNL(*(int16_t *)param.paramPtr);
    }
    void LogParamI8(Param &param)
    {
        LogNNL(*(int8_t *)param.paramPtr);
    }

    
private:

    SerialAsyncConsole<0, MAX_LINE_LEN>  console_;

    Param paramList_[PARAM_COUNT];
    uint8_t paramListIdx_ = 0;
    
    Command commandList_[CMD_COUNT];
    uint8_t commandListIdx_ = 0;
    
    function<void(void)> cbFnOnSet_;
    
    function<void(Param &param)> fnFormatterList_[FORMATTER_COUNT];
    uint8_t fnFormatterListIdx_ = 0;
};



#include "Eeprom.h"

// Inherit from this.
// Provide the type you want to be able to set values on.
// Then decide the number of parameters you intend to expose.
// If you want any commands, indicate those as well.
template <typename T, uint8_t PARAM_COUNT, uint8_t CMD_COUNT, uint8_t FORMATTER_COUNT>
class PersistantConfigManager
{
public:

    PersistantConfigManager(uint8_t pinConfigure, T &config)
    : pinConfigure_(pinConfigure)
    , config_(config)
    , saved_(0)
    {
        // Nothing to do
    }
    
    uint8_t GetConfig(uint8_t letDefaultApplyAutomatically = 0)
    {
        uint8_t retVal = 0;
        
        if (letDefaultApplyAutomatically || DetectUserWantsToConfigure())
        {
            retVal = InteractivelyConfigure(letDefaultApplyAutomatically);
        }
        else
        {
            LogNNL(P("No configure signal -- "));
            
            retVal = ea_.Read(Config());
            
            if (retVal)
            {
                Log(P("OK: prior config found"));
                LogNL();
                
                SetupMenu();
                menu_.ShowParams();
                LogNL();
            }
            else
            {
                Log(P("ERR: no prior config found"));
            }
        }
        
        return retVal;
    }
    
    
private:

    uint8_t DetectUserWantsToConfigure()
    {
        uint8_t retVal = 0;
        
        // Force pin into a known high state.
        // The only way this pin could be low would be if something was pulling
        // it low.
        // That is our signal.
        PAL.PinMode(pinConfigure_, INPUT_PULLUP);
        
        LogNNL(P("Ground to configure "));
        
        uint32_t timeStart   = PAL.Millis();
        uint32_t timeLastDot = timeStart;
        
        uint8_t cont = 1;
        while (cont)
        {
            uint32_t timeNow = PAL.Millis();
            
            if (timeNow - timeLastDot >= 1000)
            {
                timeLastDot = timeLastDot + 1000;
                
                LogNNL('.');
            }
            
            if (PAL.DigitalRead(pinConfigure_) == 0)
            {
                retVal = 1;
                
                cont = 0;
            }
            else if (timeNow - timeStart >= 5000)
            {
                cont = 0;
            }
        }
        LogNL();
        
        return retVal;
    }
    
    uint8_t InteractivelyConfigure(uint8_t letDefaultApplyAutomatically)
    {
        uint8_t retVal = 0;
        
        uint8_t userConfigAcquired = ea_.Read(Config());
        
        if (userConfigAcquired)
        {
            Log(P("Prior config loaded"));
        }
        else
        {
            Log(P("No prior config, defaulting"));
        }
        LogNL();
        
        saved_ = 0;
        
        // Add our own command 
        Menu().RegisterCommand(P("done"), [this](){
            OnDone();
            Save();
            Evm::GetInstance().EndMainLoop();
        });
        
        // Save on change
        Menu().SetOnSetCallback([this](){
            Save();
        });
        
        // Allow inheriting object to set up its interfaces
        SetupMenu();
        
        // Display and let user interaction drive
        Menu().ShowAll();
        
        if (letDefaultApplyAutomatically)
        {
            OnDone();
            Save();
        }
        else
        {
            Menu().Start();
            Evm::GetInstance().HoldStackDangerously();
        }
        
        retVal = saved_;
        
        return retVal;
    }
    
    // No implementation, inheriting class needs to implement
    virtual void SetupMenu() = 0;

    virtual void OnDone() { }
    
    
protected:

    using MenuType = SerialAsyncConsoleMenu<PARAM_COUNT, CMD_COUNT + 1, FORMATTER_COUNT>;
    
    MenuType &Menu()
    {
        return menu_;
    }
    
    T &Config()
    {
        return config_;
    }
    
    
private:

    void Save()
    {
        LogNL();
        Log(P("Saving"));
        LogNL();
        
        ea_.Write(Config());
        ea_.Read(Config());
        
        saved_ = 1;
        
        Menu().ShowParams();
    }

    uint8_t pinConfigure_;
    
    T &config_;
    
    EepromAccessor<T> ea_;
    
    MenuType menu_;
    
    uint8_t saved_;

};































#endif  // __SERIAL_INPUT_H__

























