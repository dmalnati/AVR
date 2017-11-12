#ifndef __APP_MM_TO_GPS_LCD_H__
#define __APP_MM_TO_GPS_LCD_H__


#include "Container.h"
#include "Evm.h"
#include "TimedEventHandler.h"
#include "IdleTimeEventHandler.h"
#include "LCDFrentaly20x4.h"


struct AppMMToGpsLCDConfig
{
    uint32_t baud;
    uint8_t  i2cAddrLcd;
    
    const char *callsign;
};

class AppMMToGpsLCD
{
    static const uint8_t C_IDLE  = 1;
    static const uint8_t C_TIMED = 1;
    static const uint8_t C_INTER = 0;
    
    static const uint32_t ONE_SECOND_IN_MS = 1000;
    
    static const uint8_t BUF_BYTES = 200;


public:
    
    AppMMToGpsLCD(AppMMToGpsLCDConfig &cfg)
    : cfg_(cfg)
    , lcd_(cfg_.i2cAddrLcd)
    , secsSinceLast_(0)
    , filteredCount_(0)
    {
        
    }
    
    void Run()
    {
        Serial.begin(cfg_.baud);
        Serial.println("Starting");
        
        lcd_.Init();
        lcd_.PrintAt( 0, 0, "time: -");
        lcd_.PrintAt( 0, 1, "lat : -");
        lcd_.PrintAt( 0, 2, "lng : -");
        lcd_.PrintAt( 0, 3, "age : -");
        lcd_.PrintAt(13, 3, "f: -");
        
        ted_.SetCallback([this](){ OnSecondInterval(); });
        ted_.RegisterForTimedEventInterval(ONE_SECOND_IN_MS);
        
        ied_.SetCallback([this](){ OnSerialPoll(); });
        ied_.RegisterForIdleTimeEvent();
        
        evm_.MainLoop();
    }
    


private:

    void OnSecondInterval()
    {
        ++secsSinceLast_;
        
        PrintAge();
    }


    // String you're looking for:
    // SRC: [KD2KDD-9] DST: [APZ001-0] PATH: [WIDE1-1] DATA: /001221h2400.00S/18000.00EO359/999/A=999999      -& %,. ! ! )$,#"  "$
    //
    // The part we care about is:
    // 001221h2400.00S/18000.00E
    // 001221h   = time
    // 2400.00S  = latitude
    // 18000.00E = longitude
    
    
    void OnSerialPoll()
    {
        while (Serial.available())
        {
            uint8_t b = Serial.read();
            Serial.write(b);
            AddByteToQueue(b);
            
            if (b == '\n')
            {
                static const uint8_t BUF_SIZE = 20;
                char buf[BUF_SIZE] = { 0 };
                
                // Now look for whether this line looks like what we want
                GetBytesFromQueueAsString(0, buf, 3);
                if (!strcmp(buf, "SRC"))
                {
                    // this is the right line, confirm from me
                    
                    GetBytesFromQueueAsString(6, buf, strlen(cfg_.callsign));
                    if (!strcmp(buf, cfg_.callsign))
                    {
                        Serial.println();
                        Serial.println("PARSED");
                        
                        GetBytesFromQueueAsString(55, buf, 6);
                        PrintTime(buf);
                        Serial.println(buf);
                        
                        GetBytesFromQueueAsString(62, buf, 8);
                        PrintLat(buf);
                        Serial.println(buf);
                        
                        GetBytesFromQueueAsString(71, buf, 9);
                        PrintLng(buf);
                        Serial.println(buf);
                        
                        secsSinceLast_ = 0;
                        PrintAge();
                        
                        Serial.println();
                    }
                    else
                    {
                        ++filteredCount_;
                        
                        PrintFiltered();
                        
                        Serial.println();
                        Serial.println("FILTERED");
                        Serial.println(buf);
                        Serial.println();
                    }
                }
                
                // blow away buffer, either not the line we want or we just used
                // it.
                q_.Clear();
            }
        }
    }
    
    void GetBytesFromQueueAsString(uint8_t  idxStart,
                                   char    *buf,
                                   uint8_t  bufSize)
    {
        for (uint8_t i = 0; i < bufSize; ++i)
        {
            buf[i] = q_[idxStart + i];
        }
        
        buf[bufSize] = '\0';
    }
    
    void AddByteToQueue(uint8_t b)
    {
        if (!q_.Push(b))
        {
            uint8_t tmp;
            
            q_.Pop(tmp);
            q_.Push(b);
        }
    }
    
    /*
     * LCD display map
     *
     * 00000000001111111111
     * 01234567890123456789
     *                     
     * time: 001221h       
     * lat : 2400.00S      
     * lng : 18000.00E     
     * age : aaaaa  f: ffff
     *
     */

    void PrintTime(const char *str)
    {
        lcd_.PrintAt(6, 0, str);
    }
    
    void PrintLat(const char *str)
    {
        lcd_.PrintAt(6, 1, str);
    }
    
    void PrintLng(const char *str)
    {
        lcd_.PrintAt(6, 2, str);
    }
    
    void PrintAge()
    {
        lcd_.PrintAt(6, 3, "     ");
        lcd_.PrintAt(6, 3, secsSinceLast_);
    }
    
    void PrintFiltered()
    {
        lcd_.PrintAt(16, 3, "    ");
        lcd_.PrintAt(16, 3, filteredCount_);
    }
    
    
private:
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;
    
    AppMMToGpsLCDConfig &cfg_;

    LCDFrentaly20x4 lcd_;
    
    uint32_t secsSinceLast_;
    uint32_t filteredCount_;
    
    TimedEventHandlerDelegate ted_;
    IdleTimeEventHandlerDelegate ied_;
    
    Queue<uint8_t, BUF_BYTES> q_;
};





#endif  // __APP_MM_TO_GPS_LCD_H__



