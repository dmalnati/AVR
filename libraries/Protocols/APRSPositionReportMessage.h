#ifndef __APRS_POSITION_REPORT_MESSAGE_H__
#define __APRS_POSITION_REPORT_MESSAGE_H__


class APRSPositionReportMessage
{
    // Minimum size of valid message with empty comment section
    static const uint8_t MIN_BUF_SIZE = 27;
    
    // Maximum size of valid message with full comment section (+ 43 bytes)
    static const uint8_t MAX_BUF_SIZE = 70;
   
public:
 
    uint8_t SetTargetBuf(uint8_t *buf, uint8_t bufSize)
    {
        uint8_t retVal = 0;
       
        if (buf && bufSize >= MIN_BUF_SIZE)
        {
            retVal = 1;
            
            buf_             = (char *)buf;
            bufSize_         = bufSize <= MAX_BUF_SIZE ? bufSize : MAX_BUF_SIZE;
            commentNextByte_ = &(buf_[MIN_BUF_SIZE]);

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
        const uint8_t BUF_LOCATION = 1;
        
        if (buf_)
        {
            // Constrain values
            if (hours   > 23) { hours   = 23; }
            if (minutes > 59) { minutes = 59; }
            if (seconds > 59) { seconds = 59; }
            
            // Get location in buffer
            char *p = &(buf_[BUF_LOCATION]);
            
            // Fill out buffer
            U32ToStrPadLeft(p, hours, 2, '0');
            p += 2;
           
            U32ToStrPadLeft(p, minutes, 2, '0');
            p += 2;
           
            U32ToStrPadLeft(p, seconds, 2, '0');
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
        const uint8_t BUF_LOCATION = 8;
        
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
            U32ToStrPadLeft(p, degreesPos, 2, '0');
            p += 2;
           
            U32ToStrPadLeft(p, minutes, 2, '0');
            p += 2;
           
            *p = '.';
            p += 1;
           
            U32ToStrPadLeft(p, secondsAsHundredths, 2, '0');
            p += 2;
           
            *p = northOrSouth;
        }
    }
    
    void SetSymbolTableID(char id)
    {
        const uint8_t BUF_LOCATION = 16;
        
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
        const uint8_t BUF_LOCATION = 17;
        
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
            U32ToStrPadLeft(p, degreesPos, 3, '0');
            p += 3;
           
            U32ToStrPadLeft(p, minutes, 2, '0');
            p += 2;
           
            *p = '.';
            p += 1;
           
            U32ToStrPadLeft(p, secondsAsHundredths, 2, '0');
            p += 2;
           
            *p = eastOrWest;
        }
    }
    
    void SetSymbolCode(char code)
    {
        const uint8_t BUF_LOCATION = 26;
        
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
        
        if (!CanFitCommentBytes(strLen))
        {
            bytesToCopy = GetCommentBytesRemaining();
        }
        
        memcpy(commentNextByte_, str, bytesToCopy);
        commentNextByte_ += bytesToCopy;
    }
    
    void AppendCommentU32PadLeft(uint32_t val,
                                 uint8_t  width,
                                 uint32_t limLower,
                                 uint32_t limUpper)
    {
        // Check bounds
        if (width <= GetCommentBytesRemaining())
        {
            // Constrain values
            if (val < limLower) { val = limLower; }
            if (val > limUpper) { val = limUpper; }
            
            // Fill out buffer
            U32ToStrPadLeft(commentNextByte_, val, width, '0');
            
            commentNextByte_ += width;
        }
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
        const uint8_t WIDTH = 7;
        
        if (WIDTH <= GetCommentBytesRemaining())
        {
            AppendCommentU32PadLeft(course, 3, 1, 360);
            AppendCommentString("/");
            AppendCommentU32PadLeft(speed, 3, 0, 999);
        }
    }
    

    //
    // 9-character field
    // Altitude in feet
    //
    void SetCommentAltitude(uint32_t altitude)
    {
        const uint8_t WIDTH = 9;
        
        if (WIDTH <= GetCommentBytesRemaining())
        {
            AppendCommentString("/A=");
            AppendCommentU32PadLeft(altitude, 6, 0, 999999);
        }
    }


private:

    void SetMessageType()
    {
        const uint8_t BUF_LOCATION = 0;
        
        // Get location in buffer
        char *p = &(buf_[BUF_LOCATION]);
        
        // Set MessageType (no messaging, w/ timestamp)
        *p = '/';
    }
    
    uint8_t GetCommentBytesRemaining()
    {
        uint8_t retVal = 0;
        
        if (buf_)
        {
            uint8_t commentBytesAllocated = (bufSize_ - MIN_BUF_SIZE);
            uint8_t commentBytesUsed      = (commentNextByte_ - &(buf_[MIN_BUF_SIZE]));
            
            retVal = commentBytesAllocated - commentBytesUsed;    
        }
        
        return retVal;
    }
    
    uint8_t CanFitCommentBytes(uint8_t byteCount)
    {
        return byteCount <= GetCommentBytesRemaining();
    }
    
    void U32ToStrPadLeft(char *bufTarget, uint8_t val, uint8_t width, char pad)
    {
        // Allocate local space so the NULL terminator doesn't spill beyond the
        // width specified in the target buffer.
        char buf[width + 1];
     
        // Convert to ASCII representation, plus NULL, base 10
        ultoa(val, buf, 10);
       
        // Shift ASCII chars to the right and pad if necessary
        uint8_t len = strlen(buf);
       
        if (len < width)
        {
            memmove(&buf[width - len], buf, len + 1);
            memset(buf, pad, width - len);
        }
       
        // Copy chars (not null terminator) to target buffer
        memcpy(bufTarget, buf, width);
    }


    char    *buf_             = NULL;
    uint8_t  bufSize_         = 0;
    char    *commentNextByte_ = NULL;
};



#endif  // __APRS_POSITION_REPORT_MESSAGE_H__










