#ifndef __APRS_POSITION_REPORT_MESSAGE_PICO_TRACKER_1_H__
#define __APRS_POSITION_REPORT_MESSAGE_PICO_TRACKER_1_H__


#include "APRSPositionReportMessage.h"


class APRSPositionReportMessagePicoTracker1
: public APRSPositionReportMessage
{
public:

    
    
    
    // 2-char field
    void SetCommentSeqNoBinaryEncoded(uint16_t seqNo)
    {
        AppendCommentU16Encoded(seqNo);
    }
    
    
    

private:
};


#endif  // __APRS_POSITION_REPORT_MESSAGE_PICO_TRACKER_1_H__





