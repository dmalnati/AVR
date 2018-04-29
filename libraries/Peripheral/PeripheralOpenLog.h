#ifndef __PERIPHERAL_OPEN_LOG_H__
#define __PERIPHERAL_OPEN_LOG_H__


#include "Software@fix@Serial.h"

#include "PAL.h"
#include "Container.h"


// Forward declaration
class PeripheralOpenLogFileHandle;


class PeripheralOpenLogBase
{
    friend class PeripheralOpenLogFileHandle;

protected:
    static const uint32_t DEFAULT_BAUD                   = 9600;
    static const uint16_t WARMUP_TIME_MS                 = 2000;
    
    static const uint8_t  DELAY_INPUT_TO_COMMAND_MODE_MS = 25;
    static const uint8_t  DELAY_UNKNOWN_COMMAND          = 100;
    static const uint8_t  DELAY_PREPARE_APPEND_TO_FILE   = 5;

    static const uint8_t  CHAR_CTL_Z                     = 0x1A;
    static const uint8_t  CHAR_CARRIAGE_RETURN           = '\r';
    
public:
    PeripheralOpenLogBase(int8_t pinTx, uint32_t baud = DEFAULT_BAUD)
    : ss_(PAL.GetArduinoPinFromPhysicalPin(-1),
          PAL.GetArduinoPinFromPhysicalPin(pinTx))
    , baud_(baud)
    , fileHandleActive_(NULL)
    {
        // Nothing to do
    }
    
    ~PeripheralOpenLogBase() {}
    
    void Init()
    {
        // Give the Logger a moment to warm up and do stuff in the event that
        // both are starting at the same time.
        PAL.Delay(WARMUP_TIME_MS);
        
        // Start serial comms
        ss_.begin(baud_);
        
        Reset();
    }
    
    void Reset()
    {
        // Sync and reset in the event that this system was
        // restarted but the logger kept running in an unknown state.
        Cmd("sync");
        Cmd("reset");
        EnterCommandMode();
        
        fileHandleActive_ = NULL;
    }
    
    void Cmd(const char *cmd)
    {
        EnterCommandMode();
        
        ss_.print(cmd);
        ss_.write(CHAR_CARRIAGE_RETURN);
        
        PAL.Delay(DELAY_UNKNOWN_COMMAND);
    }
    
    void PrepareToAppendToFile(const char *filename)
    {
        EnterCommandMode();
        
        ss_.print("append ");
        ss_.print(filename);
        ss_.write(CHAR_CARRIAGE_RETURN);
        
        PAL.Delay(DELAY_PREPARE_APPEND_TO_FILE);
    }
    
    template <typename T>
    void Append(T data)
    {
        ss_.print(data);
    }
    
    void Append(uint8_t *buf, uint8_t bufLen)
    {
        ss_.write(buf, bufLen);
    }
    
    void EnterCommandMode()
    {
        ss_.write(CHAR_CTL_Z);
        ss_.write(CHAR_CTL_Z);
        ss_.write(CHAR_CTL_Z);
        ss_.write(CHAR_CARRIAGE_RETURN);
        
        PAL.Delay(DELAY_INPUT_TO_COMMAND_MODE_MS);
    }

private:
    SoftwareSerial ss_;
    uint32_t       baud_;
    
    PeripheralOpenLogFileHandle *fileHandleActive_;
};



template <uint8_t MAX_FILE_HANDLES>
class PeripheralOpenLog
: public PeripheralOpenLogBase
{
public:
    PeripheralOpenLog(int8_t pinTx,
                      uint32_t baud = PeripheralOpenLogBase::DEFAULT_BAUD)
    : PeripheralOpenLogBase(pinTx, baud)
    {
        // Nothing to do
    }

    PeripheralOpenLogFileHandle *GetFileHandle(const char *filename)
    {
        PeripheralOpenLogFileHandle *fileHandle =
            fileHandleList_.PushNew(this, filename);
            
        return fileHandle;
    }
    
private:
    ListInPlace<PeripheralOpenLogFileHandle, MAX_FILE_HANDLES> fileHandleList_;
};



class PeripheralOpenLogFileHandle
{
public:
    PeripheralOpenLogFileHandle(PeripheralOpenLogBase *pol, const char *filename)
    : pol_(pol)
    , filename_(filename)
    {
        // Nothing to do
    }

    ~PeripheralOpenLogFileHandle() { }

    template <typename T>
    void Append(T data)
    {
        MakeThisFileActiveForAppending();
        
        pol_->Append(data);
    }
    
    void Append(uint8_t *buf, uint8_t bufLen)
    {
        MakeThisFileActiveForAppending();
        
        pol_->Append(buf, bufLen);
    }
    
    
private:
    void MakeThisFileActiveForAppending()
    {
        if (pol_->fileHandleActive_ != this)
        {
            // Assume that the logger is currently accepting bytes to be
            // streamed in, but not to this file.  That's not what we want.
            
            pol_->PrepareToAppendToFile(filename_);
            
            pol_->fileHandleActive_ = this;
        }
    }

    PeripheralOpenLogBase *pol_;
    const char            *filename_;
};








#endif  // __PERIPHERAL_OPEN_LOG_H__




















