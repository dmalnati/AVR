#ifndef __APRS_POSITION_REPORT_MESSAGE_PICO_TRACKER_1_H__
#define __APRS_POSITION_REPORT_MESSAGE_PICO_TRACKER_1_H__


#include "APRSPositionReportMessage.h"


class APRSPositionReportMessagePicoTracker1
: public APRSPositionReportMessage
{
public:

    void SetCommentMessageType(char msgType)
    {
        AppendCommentU8((uint8_t)msgType);
    }
    
    void SetCommentSeqNoBinaryEncoded(uint16_t seqNo)
    {
        AppendCommentU16Encoded(seqNo);
    }
    
    void SetCommentGpsLockWaitSecsBinaryEncoded(uint8_t gpsLockWaitSecs)
    {
        AppendCommentU8Encoded(gpsLockWaitSecs);
    }
    
    void SetCommentNumRestartsBinaryEncoded(uint16_t numRestarts)
    {
        AppendCommentU16Encoded(numRestarts);
    }
    
    void SetCommentNumWdtRestartsBinaryEncoded(uint16_t numWdtRestarts)
    {
        AppendCommentU16Encoded(numWdtRestarts);
    }
    
    void SetCommentNumMsgsNotSentBinaryEncoded(uint16_t numMsgsNotSent)
    {
        AppendCommentU16Encoded(numMsgsNotSent);
    }
    

private:
};


#endif  // __APRS_POSITION_REPORT_MESSAGE_PICO_TRACKER_1_H__





