#ifndef __UTL_SERIAL_H__
#define __UTL_SERIAL_H__


#include "Serial.h"
#include "Log.h"
#include "Function.h"
#include "Str.h"
#include "StreamWindow.h"
#include "TimedEventHandler.h"




class SerialAsyncReadLine
{
    static const uint32_t DEFAULT_POLL_PERIOD_MS = 50;
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
    
    void SetVerbose(uint8_t verbose)
    {
        verbose_ = verbose;
    }
    
    void Start()
    {
        isRunning_ = 1;
        
        sarl_.Attach(buf_, BUF_SIZE);
        sarl_.SetCallback([this](char *str){
            uint8_t strLen = strlen(str);
            
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
                Log(cmdToFnList_[i].cmd);
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
: public SerialAsyncConsole<NUM_COMMANDS + 2, NUM_BYTES_SERIAL>
{
public:

    SerialAsyncConsoleEnhanced(uint8_t enableDefaultErrorHandler = 1)
    {
        Init(enableDefaultErrorHandler);
    }

    
private:

    void Init(uint8_t enableDefaultErrorHandler)
    {
        // two sub-commands:
        // - pin set <pin> <val>
        // - pin get <pin>
        this->RegisterCommand("pin", [](char *cmdStr) {
            Str str(cmdStr);
            
            if (str.TokenCount(' ') == 4 &&
                !strcmp_P(str.TokenAtIdx(1, ' '), PSTR("set")))
            {
                uint8_t pin = atoi(str.TokenAtIdx(2, ' '));
                uint8_t val = atoi(str.TokenAtIdx(3, ' '));

                PAL.PinMode(pin, OUTPUT);
                
                Log(P("Pin "), pin, P(" -> "), val);

                PAL.DigitalWrite(pin, val);
            }
            else if (str.TokenCount(' ') == 3 &&
                !strcmp_P(str.TokenAtIdx(1, ' '), PSTR("get")))
            {
                uint8_t pin = atoi(str.TokenAtIdx(2, ' '));
                
                PAL.PinMode(pin, INPUT);
                
                uint8_t val = PAL.DigitalRead(pin);

                Log(P("Pin "), pin, P(" <- "), val);
            }
        });
        
        if (enableDefaultErrorHandler)
        {
            this->RegisterErrorHandler([](char *cmdStr) {
                Log(P("ERR: \""), cmdStr, '\"');
            });
        }
    }
};




uint8_t SerialReadLine(char *buf, uint8_t bufSize)
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
            uint8_t strLen = SerialReadLine(buf_, BUF_SIZE);

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


#endif  // __UTL_SERIAL_H__

























