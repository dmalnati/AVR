#ifndef __APRS_POSITION_REPORT_MESSAGE_H__
#define __APRS_POSITION_REPORT_MESSAGE_H__


#include "StrFormat.h"


class APRSPositionReportMessageBase
{
protected:

    struct MessageDetails
    {
        uint8_t MIN_BUF_SIZE = 0;
        uint8_t MAX_BUF_SIZE = 0;

        char messageType = '\0';

        uint8_t idxTime       = 0; // size 7
        uint8_t idxLat        = 0; // size 8
        uint8_t idxSymTableId = 0; // size 1
        uint8_t idxLng        = 0; // size 9
        uint8_t idxSymCode    = 0; // size 1
    };

    MessageDetails msgDetails_;


public:
 
    uint8_t SetTargetBuf(uint8_t *buf, uint8_t bufSize)
    {
        uint8_t retVal = 0;
       
        if (buf && bufSize >= msgDetails_.MIN_BUF_SIZE)
        {
            retVal = 1;
            
            buf_             = (char *)buf;
            bufSize_         = bufSize <= msgDetails_.MAX_BUF_SIZE ? bufSize : msgDetails_.MAX_BUF_SIZE;
            commentNextByte_ = &(buf_[msgDetails_.MIN_BUF_SIZE]);

            // Zero (spaces) all buf bytes passed in
            memset(buf_, ' ', bufSize);

            // Place markers
            SetMessageType();
        }
        else
        {
            buf_             = NULL;
            bufSize_         = 0;
            commentNextByte_ = NULL;
        }
       
        return retVal;
    }
    
    uint8_t GetBytesUsed()
    {
        uint8_t retVal = 0;
        
        if (buf_)
        {
            retVal = (commentNextByte_ - buf_);
        }
        
        return retVal;
    }
    
    
    // 7-character string
    //
    // Format as HHMMSSh (where the 'h' is literal)
    // 24-hour time
    void SetTimeLocal(uint8_t hours, uint8_t minutes, uint8_t seconds)
    {
        const uint8_t BUF_LOCATION = msgDetails_.idxTime;
        
        if (buf_)
        {
            // Constrain values
            if (hours   > 23) { hours   = 23; }
            if (minutes > 59) { minutes = 59; }
            if (seconds > 59) { seconds = 59; }
            
            // Get location in buffer
            char *p = &(buf_[BUF_LOCATION]);
            
            // Fill out buffer
            StrFormat::U32ToStrPadLeft(p, hours, 2, '0');
            p += 2;
           
            StrFormat::U32ToStrPadLeft(p, minutes, 2, '0');
            p += 2;
           
            StrFormat::U32ToStrPadLeft(p, seconds, 2, '0');
            p += 2;
           
            *p = 'h';
        }
    }
   
    //
    // The latitude is shown as the 8-character string
    // ddmm.hhN (i.e. degrees, minutes and hundredths of a minute north)
    //
    // ex: 4903.50N is 49 degrees 3 minutes 30 seconds north
    //
    void SetLatitude(int8_t degrees, uint8_t minutes, double seconds)
    {
        const uint8_t BUF_LOCATION = msgDetails_.idxLat;
        
        if (buf_)
        {
            // Constrain values
            if (degrees < -90) { degrees = -90; }
            if (degrees >  90) { degrees =  90; }
            if (minutes >  59) { minutes =  59; }
            if (seconds >  59) { seconds =  59; }
           
            // Calculate values
            char    northOrSouth        = (degrees >= 0 ? 'N' : 'S');
            uint8_t degreesPos          = fabs(degrees);
            uint8_t secondsAsHundredths = round(seconds / 60.0 * 100.0);
            
            if (secondsAsHundredths == 100) { secondsAsHundredths = 99; }
           
            // Get location in buffer
            char *p = &(buf_[BUF_LOCATION]);
           
            // Fill out buffer
            StrFormat::U32ToStrPadLeft(p, degreesPos, 2, '0');
            p += 2;
           
            StrFormat::U32ToStrPadLeft(p, minutes, 2, '0');
            p += 2;
           
            *p = '.';
            p += 1;
           
            StrFormat::U32ToStrPadLeft(p, secondsAsHundredths, 2, '0');
            p += 2;
           
            *p = northOrSouth;
        }
    }
    
    void SetSymbolTableID(char id)
    {
        const uint8_t BUF_LOCATION = msgDetails_.idxSymTableId;
        
        if (buf_)
        {
            // Get location in buffer
            char *p = &(buf_[BUF_LOCATION]);
            
            // Set ID
            *p = id;
        }
    }
    
    //
    // The longitude is shown as the 9-character string
    // dddmm.hhW (i.e. degrees, minutes and hundredths of a minute west)
    //
    // ex: 07201.75W is 72 degrees 1 minute 45 seconds west
    //
    void SetLongitude(int16_t degrees, uint8_t minutes, double seconds)
    {
        const uint8_t BUF_LOCATION = msgDetails_.idxLng;
        
        if (buf_)
        {
            // Constrain values
            if (degrees < -179) { degrees = -179; }
            if (degrees >  180) { degrees =  180; }
            if (minutes >   59) { minutes =   59; }
            if (seconds >   59) { seconds =   59; }
            
            // Calculate values
            char    eastOrWest          = (degrees >= 0 ? 'E' : 'W');
            uint8_t degreesPos          = fabs(degrees);
            uint8_t secondsAsHundredths = round(seconds / 60.0 * 100.0);
            
            if (secondsAsHundredths == 100) { secondsAsHundredths = 99; }
           
            // Get location in buffer
            char *p = &(buf_[BUF_LOCATION]);
           
            // Fill out buffer
            StrFormat::U32ToStrPadLeft(p, degreesPos, 3, '0');
            p += 3;
           
            StrFormat::U32ToStrPadLeft(p, minutes, 2, '0');
            p += 2;
           
            *p = '.';
            p += 1;
           
            StrFormat::U32ToStrPadLeft(p, secondsAsHundredths, 2, '0');
            p += 2;
           
            *p = eastOrWest;
        }
    }
    
    void SetSymbolCode(char code)
    {
        const uint8_t BUF_LOCATION = msgDetails_.idxSymCode;
        
        if (buf_)
        {
            // Get location in buffer
            char *p = &(buf_[BUF_LOCATION]);
            
            // Set code
            *p = code;
        }
    }
    
    
    //
    // Helper functions to append data to the Comment section.
    //
    
    void AppendCommentString(const char *str)
    {
        uint8_t strLen      = strlen(str);
        uint8_t bytesToCopy = strLen;
        
        if (buf_)
        {
            if (!CanFitCommentBytes(strLen))
            {
                bytesToCopy = GetCommentBytesRemaining();
            }
            
            memcpy(commentNextByte_, str, bytesToCopy);
            commentNextByte_ += bytesToCopy;
        }
    }
    
    // left justified, space padded
    void AppendCommentStringFixedWidth(const char *str, uint8_t width)
    {
        if (buf_)
        {
            uint8_t strLen      = strlen(str);
            uint8_t bytesToCopy = strLen < width ? strLen : width;
            
            // copy in the bytes from the string which fits
            if (!CanFitCommentBytes(bytesToCopy))
            {
                bytesToCopy = GetCommentBytesRemaining();
            }
            memcpy(commentNextByte_, str, bytesToCopy);
            commentNextByte_ += bytesToCopy;
            
            // copy in whatever padding fits
            uint8_t padBytes = width - bytesToCopy;
            if (!CanFitCommentBytes(padBytes))
            {
                padBytes = GetCommentBytesRemaining();
            }
            
            memset(commentNextByte_, ' ', padBytes);
            commentNextByte_ += padBytes;
        }
    }
    
    void AppendCommentU32PadLeft(uint32_t val,
                                 uint8_t  width,
                                 uint32_t limLower,
                                 uint32_t limUpper)
    {
        // Check bounds
        if (buf_ && width <= GetCommentBytesRemaining())
        {
            // Constrain values
            if (val < limLower) { val = limLower; }
            if (val > limUpper) { val = limUpper; }
            
            // Fill out buffer
            StrFormat::U32ToStrPadLeft(commentNextByte_, val, width, '0');
            
            commentNextByte_ += width;
        }
    }
    
    
    void AppendCommentU8(uint8_t val)
    {
        if (buf_ && sizeof(val) <= GetCommentBytesRemaining())
        {
            memcpy(commentNextByte_, &val, sizeof(val));
            
            commentNextByte_ += sizeof(val);
        }
    }
    
    void AppendCommentI8Encoded(int8_t val)
    {
        AppendCommentU8Encoded(val);
    }
    
    void AppendCommentU8Encoded(uint8_t val)
    {
        if (buf_ && (sizeof(val) * 2) <= GetCommentBytesRemaining())
        {
            uint8_t b1 = (uint8_t)((val & 0b11110000) >> 4) + 32;
            uint8_t b2 = (uint8_t) (val & 0b00001111)       + 32;
            
            AppendCommentU8(b1);
            AppendCommentU8(b2);
        }
    }
    
    void AppendCommentU16Encoded(uint16_t val)
    {
        uint16_t valBigEndian = PAL.htons(val);
        
        uint8_t *p = (uint8_t *)&valBigEndian;
        
        AppendCommentU8Encoded(p[0]);
        AppendCommentU8Encoded(p[1]);
    }
    
    
    //
    // Helper functions for setting standard extensions to the Comment
    // section.
    //
    // If these are used, they need to be used in their listed order, and before
    // any other appending to the Comment section.
    //
    
    
    //
    // 7-character field
    // CCC/SSS
    // Course 1-360 degrees
    // Speed in knots
    //
    void SetCommentCourseAndSpeed(uint16_t course, uint16_t speed)
    {
        AppendCommentU32PadLeft(course, 3, 0, 359);
        AppendCommentString("/");
        AppendCommentU32PadLeft(speed, 3, 0, 999);
    }
    

    //
    // 9-character field
    // Altitude in feet
    //
    void SetCommentAltitude(uint32_t altitude)
    {
        AppendCommentString("/A=");
        AppendCommentU32PadLeft(altitude, 6, 0, 999999);
    }


private:

    void SetMessageType()
    {
        const uint8_t BUF_LOCATION = 0;
        
        if (buf_)
        {
            // Get location in buffer
            char *p = &(buf_[BUF_LOCATION]);
            
            // Set MessageType
            *p = msgDetails_.messageType;
        }
    }
    
    uint8_t GetCommentBytesRemaining()
    {
        uint8_t retVal = 0;
        
        if (buf_)
        {
            uint8_t commentBytesAllocated = (bufSize_ - msgDetails_.MIN_BUF_SIZE);
            uint8_t commentBytesUsed      = (commentNextByte_ - &(buf_[msgDetails_.MIN_BUF_SIZE]));
            
            retVal = commentBytesAllocated - commentBytesUsed;    
        }
        
        return retVal;
    }
    
    uint8_t CanFitCommentBytes(uint8_t byteCount)
    {
        return byteCount <= GetCommentBytesRemaining();
    }

    
    
    char    *buf_             = NULL;
    uint8_t  bufSize_         = 0;
    char    *commentNextByte_ = NULL;
};


// p. 94 (pdf page 104)
class APRSPositionReportMessage
: public APRSPositionReportMessageBase
{
public:

    APRSPositionReportMessage()
    {
        msgDetails_ = {
            // Minimum size of valid message with empty comment section
            .MIN_BUF_SIZE = 27,
            // Maximum size of valid message with full comment section (+ 43 bytes)
            .MAX_BUF_SIZE = 70,

            .messageType = '/',

            .idxTime       =  1, // size 7
            .idxLat        =  8, // size 8
            .idxSymTableId = 16, // size 1
            .idxLng        = 17, // size 9
            .idxSymCode    = 26, // size 1
        };
    }
};


// p. 94 (pdf page 104)
class APRSPositionReportMessageNoTimestamp
: public APRSPositionReportMessageBase
{
public:

    APRSPositionReportMessageNoTimestamp()
    {
        msgDetails_ = {
            // Minimum size of valid message with empty comment section
            .MIN_BUF_SIZE = 20,
            // Maximum size of valid message with full comment section (+ 43 bytes)
            .MAX_BUF_SIZE = 63,

            .messageType = '=',

            .idxTime       =  0, // doesn't exist
            .idxLat        =  1, // size 8
            .idxSymTableId =  9, // size 1
            .idxLng        = 10, // size 9
            .idxSymCode    = 19, // size 1
        };
    }
};




#endif  // __APRS_POSITION_REPORT_MESSAGE_H__










