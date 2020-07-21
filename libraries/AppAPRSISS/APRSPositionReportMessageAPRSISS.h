#ifndef __APRS_POSITION_REPORT_MESSAGE_APRS_ISS_H__
#define __APRS_POSITION_REPORT_MESSAGE_APRS_ISS_H__


#include "APRSPositionReportMessage.h"


class APRSPositionReportMessageAPRSISS
: public APRSPositionReportMessageNoTimestamp
{
public:

    void SetComment(char *comment)
    {
        AppendCommentString(comment);
    }


private:
};


#endif  // __APRS_POSITION_REPORT_MESSAGE_APRS_ISS_H__





