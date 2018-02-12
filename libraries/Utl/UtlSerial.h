#ifndef __UTL_SERIAL_H__
#define __UTL_SERIAL_H__


#include "Function.h"
#include "Str.h"


uint8_t SerialReadLine(char *buf, uint8_t bufSize)
{
    uint8_t retVal = 0;
 
    while (!retVal)
    {
        retVal = Serial.readBytesUntil('\n', buf, bufSize);

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
                if (Serial.available())
                {
                    char c = Serial.read();
                    
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
        Serial.println(F("Serial Commands:"));
        Serial.println(F("----------------"));
        for (uint8_t i = 0; i < cmdToFnListIdx_; ++i)
        {
            Serial.println(cmdToFnList_[i].cmd);
        }
        Serial.println();

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
                
                Serial.println();
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

























