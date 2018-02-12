////////////////////////////////////////////////////////////////////
//
// COMMAND PART_INFO (0x01)
//
////////////////////////////////////////////////////////////////////

struct PART_INFO_REP
{
    struct
    {
        uint8_t CHIPREV  = 0;
    } CHIPREV;

    struct
    {
        uint16_t PART     = 0;
    } PART;

    struct
    {
        uint8_t PBUILD   = 0;
    } PBUILD;

    struct
    {
        uint16_t ID       = 0;
    } ID;

    struct
    {
        uint8_t CUSTOMER = 0;
    } CUSTOMER;

    struct
    {
        uint8_t ROMID    = 0;
    } ROMID;
};

uint8_t Command_PART_INFO(PART_INFO_REP &rep)
{
    const uint8_t CMD_ID       = 0x01;
    const uint8_t BUF_SIZE_REQ = 0;
    const uint8_t BUF_SIZE_REP = 8;

    uint8_t bufReq[BUF_SIZE_REQ];
    uint8_t bufRep[BUF_SIZE_REP];

    uint8_t ok = SendAndWaitAndReceive(CMD_ID, bufReq, BUF_SIZE_REQ, bufRep, BUF_SIZE_REP);

    if (ok)
    {
        BufferFieldExtractor bfe(bufRep, BUF_SIZE_REP);

        rep.CHIPREV.CHIPREV   = bfe.GetUI8();

        rep.PART.PART         = bfe.GetUI16NTOHS();

        rep.PBUILD.PBUILD     = bfe.GetUI8();

        rep.ID.ID             = bfe.GetUI16NTOHS();

        rep.CUSTOMER.CUSTOMER = bfe.GetUI8();

        rep.ROMID.ROMID       = bfe.GetUI8();
    }

    return ok;
}

////////////////////////////////////////////////////////////////////
//
// COMMAND FUNC_INFO (0x10)
//
////////////////////////////////////////////////////////////////////

struct FUNC_INFO_REP
{
    struct
    {
        uint8_t REVEXT    = 0;
    } REVEXT;

    struct
    {
        uint8_t REVBRANCH = 0;
    } REVBRANCH;

    struct
    {
        uint8_t REVINT    = 0;
    } REVINT;

    struct
    {
        uint16_t PATCH     = 0;
    } PATCH;

    struct
    {
        uint8_t FUNC      = 0;
    } FUNC;
};

uint8_t Command_FUNC_INFO(FUNC_INFO_REP &rep)
{
    const uint8_t CMD_ID       = 0x10;
    const uint8_t BUF_SIZE_REQ = 0;
    const uint8_t BUF_SIZE_REP = 6;

    uint8_t bufReq[BUF_SIZE_REQ];
    uint8_t bufRep[BUF_SIZE_REP];

    uint8_t ok = SendAndWaitAndReceive(CMD_ID, bufReq, BUF_SIZE_REQ, bufRep, BUF_SIZE_REP);

    if (ok)
    {
        BufferFieldExtractor bfe(bufRep, BUF_SIZE_REP);

        rep.REVEXT.REVEXT       = bfe.GetUI8();

        rep.REVBRANCH.REVBRANCH = bfe.GetUI8();

        rep.REVINT.REVINT       = bfe.GetUI8();

        rep.PATCH.PATCH         = bfe.GetUI16NTOHS();

        rep.FUNC.FUNC           = bfe.GetUI8();
    }

    return ok;
}

////////////////////////////////////////////////////////////////////
//
// COMMAND GET_INT_STATUS (0x20)
//
////////////////////////////////////////////////////////////////////

struct GET_INT_STATUS_REQ
{
    struct
    {
        uint8_t FILTER_MATCH_PEND_CLR                  = 0;
        uint8_t FILTER_MISS_PEND_CLR                   = 0;
        uint8_t PACKET_SENT_PEND_CLR                   = 0;
        uint8_t PACKET_RX_PEND_CLR                     = 0;
        uint8_t CRC_ERROR_PEND_CLR                     = 0;
        uint8_t ZERO                                   = 0;
        uint8_t TX_FIFO_ALMOST_EMPTY_PEND_CLR          = 0;
        uint8_t RX_FIFO_ALMOST_FULL_PEND_CLR           = 0;
    } PH_CLR_PEND;

    struct
    {
        uint8_t ZERO                                   = 0;
        uint8_t INVALID_SYNC_PEND_CLR                  = 0;
        uint8_t RSSI_JUMP_PEND_CLR                     = 0;
        uint8_t RSSI_PEND_CLR                          = 0;
        uint8_t INVALID_PREAMBLE_PEND_CLR              = 0;
        uint8_t PREAMBLE_DETECT_PEND_CLR               = 0;
        uint8_t SYNC_DETECT_PEND_CLR                   = 0;
    } MODEM_CLR_PEND;

    struct
    {
        uint8_t CAL_PEND_CLR                           = 0;
        uint8_t FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND_CLR = 0;
        uint8_t STATE_CHANGE_PEND_CLR                  = 0;
        uint8_t CMD_ERROR_PEND_CLR                     = 0;
        uint8_t CHIP_READY_PEND_CLR                    = 0;
        uint8_t LOW_BATT_PEND_CLR                      = 0;
        uint8_t WUT_PEND_CLR                           = 0;
    } CHIP_CLR_PEND;
};

struct GET_INT_STATUS_REP
{
    struct
    {
        uint8_t XXXXX                              = 0;
        uint8_t CHIP_INT_STATUS_PEND               = 0;
        uint8_t MODEM_INT_STATUS_PEND              = 0;
        uint8_t PH_INT_STATUS_PEND                 = 0;
    } INT_PEND;

    struct
    {
        uint8_t XXXXX                              = 0;
        uint8_t CHIP_INT_STATUS                    = 0;
        uint8_t MODEM_INT_STATUS                   = 0;
        uint8_t PH_INT_STATUS                      = 0;
    } INT_STATUS;

    struct
    {
        uint8_t FILTER_MATCH_PEND                  = 0;
        uint8_t FILTER_MISS_PEND                   = 0;
        uint8_t PACKET_SENT_PEND                   = 0;
        uint8_t PACKET_RX_PEND                     = 0;
        uint8_t CRC_ERROR_PEND                     = 0;
        uint8_t X                                  = 0;
        uint8_t TX_FIFO_ALMOST_EMPTY_PEND          = 0;
        uint8_t RX_FIFO_ALMOST_FULL_PEND           = 0;
    } PH_PEND;

    struct
    {
        uint8_t FILTER_MATCH                       = 0;
        uint8_t FILTER_MISS                        = 0;
        uint8_t PACKET_SENT                        = 0;
        uint8_t PACKET_RX                          = 0;
        uint8_t CRC_ERROR                          = 0;
        uint8_t X                                  = 0;
        uint8_t TX_FIFO_ALMOST_EMPTY               = 0;
        uint8_t RX_FIFO_ALMOST_FULL                = 0;
    } PH_STATUS;

    struct
    {
        uint8_t XX                                 = 0;
        uint8_t INVALID_SYNC_PEND                  = 0;
        uint8_t RSSI_JUMP_PEND                     = 0;
        uint8_t RSSI_PEND                          = 0;
        uint8_t INVALID_PREAMBLE_PEND              = 0;
        uint8_t PREAMBLE_DETECT_PEND               = 0;
        uint8_t SYNC_DETECT_PEND                   = 0;
    } MODEM_PEND;

    struct
    {
        uint8_t XX                                 = 0;
        uint8_t INVALID_SYNC                       = 0;
        uint8_t RSSI_JUMP                          = 0;
        uint8_t RSSI                               = 0;
        uint8_t INVALID_PREAMBLE                   = 0;
        uint8_t PREAMBLE_DETECT                    = 0;
        uint8_t SYNC_DETECT                        = 0;
    } MODEM_STATUS;

    struct
    {
        uint8_t CAL_PEND                           = 0;
        uint8_t FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND = 0;
        uint8_t STATE_CHANGE_PEND                  = 0;
        uint8_t CMD_ERROR_PEND                     = 0;
        uint8_t CHIP_READY_PEND                    = 0;
        uint8_t LOW_BATT_PEND                      = 0;
        uint8_t WUT_PEND                           = 0;
    } CHIP_PEND;

    struct
    {
        uint8_t CAL                                = 0;
        uint8_t FIFO_UNDERFLOW_OVERFLOW_ERROR      = 0;
        uint8_t STATE_CHANGE                       = 0;
        uint8_t CMD_ERROR                          = 0;
        uint8_t CHIP_READY                         = 0;
        uint8_t LOW_BATT                           = 0;
        uint8_t WUT                                = 0;
    } CHIP_STATUS;
};

uint8_t Command_GET_INT_STATUS(GET_INT_STATUS_REQ &req, GET_INT_STATUS_REP &rep)
{
    const uint8_t CMD_ID       = 0x20;
    const uint8_t BUF_SIZE_REQ = 3;
    const uint8_t BUF_SIZE_REP = 8;

    uint8_t bufReq[BUF_SIZE_REQ];
    uint8_t bufRep[BUF_SIZE_REP];

    // pack request data into buffer
    uint8_t tmpReqByte0 = 0;
    tmpReqByte0 |= (uint8_t)((req.PH_CLR_PEND.FILTER_MATCH_PEND_CLR                    & 0b00000001) << 7);
    tmpReqByte0 |= (uint8_t)((req.PH_CLR_PEND.FILTER_MISS_PEND_CLR                     & 0b00000001) << 6);
    tmpReqByte0 |= (uint8_t)((req.PH_CLR_PEND.PACKET_SENT_PEND_CLR                     & 0b00000001) << 5);
    tmpReqByte0 |= (uint8_t)((req.PH_CLR_PEND.PACKET_RX_PEND_CLR                       & 0b00000001) << 4);
    tmpReqByte0 |= (uint8_t)((req.PH_CLR_PEND.CRC_ERROR_PEND_CLR                       & 0b00000001) << 3);
    tmpReqByte0 |= (uint8_t)((req.PH_CLR_PEND.ZERO                                     & 0b00000001) << 2);
    tmpReqByte0 |= (uint8_t)((req.PH_CLR_PEND.TX_FIFO_ALMOST_EMPTY_PEND_CLR            & 0b00000001) << 1);
    tmpReqByte0 |= (uint8_t)((req.PH_CLR_PEND.RX_FIFO_ALMOST_FULL_PEND_CLR             & 0b00000001) << 0);
    bufReq[0] = tmpReqByte0;

    uint8_t tmpReqByte1 = 0;
    tmpReqByte1 |= (uint8_t)((req.MODEM_CLR_PEND.ZERO                                  & 0b00000011) << 6);
    tmpReqByte1 |= (uint8_t)((req.MODEM_CLR_PEND.INVALID_SYNC_PEND_CLR                 & 0b00000001) << 5);
    tmpReqByte1 |= (uint8_t)((req.MODEM_CLR_PEND.RSSI_JUMP_PEND_CLR                    & 0b00000001) << 4);
    tmpReqByte1 |= (uint8_t)((req.MODEM_CLR_PEND.RSSI_PEND_CLR                         & 0b00000001) << 3);
    tmpReqByte1 |= (uint8_t)((req.MODEM_CLR_PEND.INVALID_PREAMBLE_PEND_CLR             & 0b00000001) << 2);
    tmpReqByte1 |= (uint8_t)((req.MODEM_CLR_PEND.PREAMBLE_DETECT_PEND_CLR              & 0b00000001) << 1);
    tmpReqByte1 |= (uint8_t)((req.MODEM_CLR_PEND.SYNC_DETECT_PEND_CLR                  & 0b00000001) << 0);
    bufReq[1] = tmpReqByte1;

    uint8_t tmpReqByte2 = 0;
    tmpReqByte2 |= (uint8_t)((req.CHIP_CLR_PEND.CAL_PEND_CLR                           & 0b00000011) << 6);
    tmpReqByte2 |= (uint8_t)((req.CHIP_CLR_PEND.FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND_CLR & 0b00000001) << 5);
    tmpReqByte2 |= (uint8_t)((req.CHIP_CLR_PEND.STATE_CHANGE_PEND_CLR                  & 0b00000001) << 4);
    tmpReqByte2 |= (uint8_t)((req.CHIP_CLR_PEND.CMD_ERROR_PEND_CLR                     & 0b00000001) << 3);
    tmpReqByte2 |= (uint8_t)((req.CHIP_CLR_PEND.CHIP_READY_PEND_CLR                    & 0b00000001) << 2);
    tmpReqByte2 |= (uint8_t)((req.CHIP_CLR_PEND.LOW_BATT_PEND_CLR                      & 0b00000001) << 1);
    tmpReqByte2 |= (uint8_t)((req.CHIP_CLR_PEND.WUT_PEND_CLR                           & 0b00000001) << 0);
    bufReq[2] = tmpReqByte2;

    uint8_t ok = SendAndWaitAndReceive(CMD_ID, bufReq, BUF_SIZE_REQ, bufRep, BUF_SIZE_REP);

    if (ok)
    {
        BufferFieldExtractor bfe(bufRep, BUF_SIZE_REP);

        uint8_t tmpByte0 = bfe.GetUI8();
        rep.INT_PEND.XXXXX                               = (uint8_t)((tmpByte0 & 0b11111000) >> 3);
        rep.INT_PEND.CHIP_INT_STATUS_PEND                = (uint8_t)((tmpByte0 & 0b00000100) >> 2);
        rep.INT_PEND.MODEM_INT_STATUS_PEND               = (uint8_t)((tmpByte0 & 0b00000010) >> 1);
        rep.INT_PEND.PH_INT_STATUS_PEND                  = (uint8_t)((tmpByte0 & 0b00000001) >> 0);

        uint8_t tmpByte1 = bfe.GetUI8();
        rep.INT_STATUS.XXXXX                             = (uint8_t)((tmpByte1 & 0b11111000) >> 3);
        rep.INT_STATUS.CHIP_INT_STATUS                   = (uint8_t)((tmpByte1 & 0b00000100) >> 2);
        rep.INT_STATUS.MODEM_INT_STATUS                  = (uint8_t)((tmpByte1 & 0b00000010) >> 1);
        rep.INT_STATUS.PH_INT_STATUS                     = (uint8_t)((tmpByte1 & 0b00000001) >> 0);

        uint8_t tmpByte2 = bfe.GetUI8();
        rep.PH_PEND.FILTER_MATCH_PEND                    = (uint8_t)((tmpByte2 & 0b10000000) >> 7);
        rep.PH_PEND.FILTER_MISS_PEND                     = (uint8_t)((tmpByte2 & 0b01000000) >> 6);
        rep.PH_PEND.PACKET_SENT_PEND                     = (uint8_t)((tmpByte2 & 0b00100000) >> 5);
        rep.PH_PEND.PACKET_RX_PEND                       = (uint8_t)((tmpByte2 & 0b00010000) >> 4);
        rep.PH_PEND.CRC_ERROR_PEND                       = (uint8_t)((tmpByte2 & 0b00001000) >> 3);
        rep.PH_PEND.X                                    = (uint8_t)((tmpByte2 & 0b00000100) >> 2);
        rep.PH_PEND.TX_FIFO_ALMOST_EMPTY_PEND            = (uint8_t)((tmpByte2 & 0b00000010) >> 1);
        rep.PH_PEND.RX_FIFO_ALMOST_FULL_PEND             = (uint8_t)((tmpByte2 & 0b00000001) >> 0);

        uint8_t tmpByte3 = bfe.GetUI8();
        rep.PH_STATUS.FILTER_MATCH                       = (uint8_t)((tmpByte3 & 0b10000000) >> 7);
        rep.PH_STATUS.FILTER_MISS                        = (uint8_t)((tmpByte3 & 0b01000000) >> 6);
        rep.PH_STATUS.PACKET_SENT                        = (uint8_t)((tmpByte3 & 0b00100000) >> 5);
        rep.PH_STATUS.PACKET_RX                          = (uint8_t)((tmpByte3 & 0b00010000) >> 4);
        rep.PH_STATUS.CRC_ERROR                          = (uint8_t)((tmpByte3 & 0b00001000) >> 3);
        rep.PH_STATUS.X                                  = (uint8_t)((tmpByte3 & 0b00000100) >> 2);
        rep.PH_STATUS.TX_FIFO_ALMOST_EMPTY               = (uint8_t)((tmpByte3 & 0b00000010) >> 1);
        rep.PH_STATUS.RX_FIFO_ALMOST_FULL                = (uint8_t)((tmpByte3 & 0b00000001) >> 0);

        uint8_t tmpByte4 = bfe.GetUI8();
        rep.MODEM_PEND.XX                                = (uint8_t)((tmpByte4 & 0b11000000) >> 6);
        rep.MODEM_PEND.INVALID_SYNC_PEND                 = (uint8_t)((tmpByte4 & 0b00100000) >> 5);
        rep.MODEM_PEND.RSSI_JUMP_PEND                    = (uint8_t)((tmpByte4 & 0b00010000) >> 4);
        rep.MODEM_PEND.RSSI_PEND                         = (uint8_t)((tmpByte4 & 0b00001000) >> 3);
        rep.MODEM_PEND.INVALID_PREAMBLE_PEND             = (uint8_t)((tmpByte4 & 0b00000100) >> 2);
        rep.MODEM_PEND.PREAMBLE_DETECT_PEND              = (uint8_t)((tmpByte4 & 0b00000010) >> 1);
        rep.MODEM_PEND.SYNC_DETECT_PEND                  = (uint8_t)((tmpByte4 & 0b00000001) >> 0);

        uint8_t tmpByte5 = bfe.GetUI8();
        rep.MODEM_STATUS.XX                              = (uint8_t)((tmpByte5 & 0b11000000) >> 6);
        rep.MODEM_STATUS.INVALID_SYNC                    = (uint8_t)((tmpByte5 & 0b00100000) >> 5);
        rep.MODEM_STATUS.RSSI_JUMP                       = (uint8_t)((tmpByte5 & 0b00010000) >> 4);
        rep.MODEM_STATUS.RSSI                            = (uint8_t)((tmpByte5 & 0b00001000) >> 3);
        rep.MODEM_STATUS.INVALID_PREAMBLE                = (uint8_t)((tmpByte5 & 0b00000100) >> 2);
        rep.MODEM_STATUS.PREAMBLE_DETECT                 = (uint8_t)((tmpByte5 & 0b00000010) >> 1);
        rep.MODEM_STATUS.SYNC_DETECT                     = (uint8_t)((tmpByte5 & 0b00000001) >> 0);

        uint8_t tmpByte6 = bfe.GetUI8();
        rep.CHIP_PEND.CAL_PEND                           = (uint8_t)((tmpByte6 & 0b11000000) >> 6);
        rep.CHIP_PEND.FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND = (uint8_t)((tmpByte6 & 0b00100000) >> 5);
        rep.CHIP_PEND.STATE_CHANGE_PEND                  = (uint8_t)((tmpByte6 & 0b00010000) >> 4);
        rep.CHIP_PEND.CMD_ERROR_PEND                     = (uint8_t)((tmpByte6 & 0b00001000) >> 3);
        rep.CHIP_PEND.CHIP_READY_PEND                    = (uint8_t)((tmpByte6 & 0b00000100) >> 2);
        rep.CHIP_PEND.LOW_BATT_PEND                      = (uint8_t)((tmpByte6 & 0b00000010) >> 1);
        rep.CHIP_PEND.WUT_PEND                           = (uint8_t)((tmpByte6 & 0b00000001) >> 0);

        uint8_t tmpByte7 = bfe.GetUI8();
        rep.CHIP_STATUS.CAL                              = (uint8_t)((tmpByte7 & 0b11000000) >> 6);
        rep.CHIP_STATUS.FIFO_UNDERFLOW_OVERFLOW_ERROR    = (uint8_t)((tmpByte7 & 0b00100000) >> 5);
        rep.CHIP_STATUS.STATE_CHANGE                     = (uint8_t)((tmpByte7 & 0b00010000) >> 4);
        rep.CHIP_STATUS.CMD_ERROR                        = (uint8_t)((tmpByte7 & 0b00001000) >> 3);
        rep.CHIP_STATUS.CHIP_READY                       = (uint8_t)((tmpByte7 & 0b00000100) >> 2);
        rep.CHIP_STATUS.LOW_BATT                         = (uint8_t)((tmpByte7 & 0b00000010) >> 1);
        rep.CHIP_STATUS.WUT                              = (uint8_t)((tmpByte7 & 0b00000001) >> 0);
    }

    return ok;
}

////////////////////////////////////////////////////////////////////
//
// COMMAND POWER_UP (0x02)
//
////////////////////////////////////////////////////////////////////

struct POWER_UP_REQ
{
    struct
    {
        uint8_t PATCH   = 0;
        uint8_t ZERO    = 0;
        uint8_t FUNC    = 0;
    } BOOT_OPTIONS;

    struct
    {
        uint8_t ZERO    = 0;
        uint8_t TCXO    = 0;
    } XTAL_OPTIONS;

    struct
    {
        uint32_t XO_FREQ = 0;
    } XO_FREQ;

};

uint8_t Command_POWER_UP(POWER_UP_REQ &req)
{
    const uint8_t CMD_ID       = 0x02;
    const uint8_t BUF_SIZE_REQ = 6;
    const uint8_t BUF_SIZE_REP = 0;

    uint8_t bufReq[BUF_SIZE_REQ];
    uint8_t bufRep[BUF_SIZE_REP];

    // pack request data into buffer
    uint8_t tmpReqByte0 = 0;
    tmpReqByte0 |= (uint8_t)((req.BOOT_OPTIONS.PATCH & 0b00000001) << 7);
    tmpReqByte0 |= (uint8_t)((req.BOOT_OPTIONS.ZERO  & 0b00000001) << 6);
    tmpReqByte0 |= (uint8_t)((req.BOOT_OPTIONS.FUNC  & 0b00111111) << 0);
    bufReq[0] = tmpReqByte0;

    uint8_t tmpReqByte1 = 0;
    tmpReqByte1 |= (uint8_t)((req.XTAL_OPTIONS.ZERO  & 0b01111111) << 1);
    tmpReqByte1 |= (uint8_t)((req.XTAL_OPTIONS.TCXO  & 0b00000001) << 0);
    bufReq[1] = tmpReqByte1;

    uint32_t tmpReqByte2_5 = PAL.htonl(req.XO_FREQ.XO_FREQ);
    memcpy(&bufReq[2], &tmpReqByte2_5, sizeof(tmpReqByte2_5));


    uint8_t ok = SendAndWaitAndReceive(CMD_ID, bufReq, BUF_SIZE_REQ, bufRep, BUF_SIZE_REP);

    return ok;
}

////////////////////////////////////////////////////////////////////
//
// COMMAND CHANGE_STATE (0x34)
//
////////////////////////////////////////////////////////////////////

struct CHANGE_STATE_REQ
{
    struct
    {
        uint8_t XXXX      = 0;
        uint8_t NEW_STATE = 0;
    } NEXT_STATE;
};

uint8_t Command_CHANGE_STATE(CHANGE_STATE_REQ &req)
{
    const uint8_t CMD_ID       = 0x34;
    const uint8_t BUF_SIZE_REQ = 1;
    const uint8_t BUF_SIZE_REP = 0;

    uint8_t bufReq[BUF_SIZE_REQ];
    uint8_t bufRep[BUF_SIZE_REP];

    // pack request data into buffer
    uint8_t tmpReqByte0 = 0;
    tmpReqByte0 |= (uint8_t)((req.NEXT_STATE.XXXX      & 0b00001111) << 4);
    tmpReqByte0 |= (uint8_t)((req.NEXT_STATE.NEW_STATE & 0b00001111) << 0);
    bufReq[0] = tmpReqByte0;

    uint8_t ok = SendAndWaitAndReceive(CMD_ID, bufReq, BUF_SIZE_REQ, bufRep, BUF_SIZE_REP);

    return ok;
}

////////////////////////////////////////////////////////////////////
//
// COMMAND GPIO_PIN_CFG (0x13)
//
////////////////////////////////////////////////////////////////////

struct GPIO_PIN_CFG_REQ
{
    struct
    {
        uint8_t X            = 0;
        uint8_t PULL_CTL     = 0;
        uint8_t GPIO_MODE    = 0;
    } GPIO0;

    struct
    {
        uint8_t X            = 0;
        uint8_t PULL_CTL     = 0;
        uint8_t GPIO_MODE    = 0;
    } GPIO1;

    struct
    {
        uint8_t X            = 0;
        uint8_t PULL_CTL     = 0;
        uint8_t GPIO_MODE    = 0;
    } GPIO2;

    struct
    {
        uint8_t X            = 0;
        uint8_t PULL_CTL     = 0;
        uint8_t GPIO_MODE    = 0;
    } GPIO3;

    struct
    {
        uint8_t X            = 0;
        uint8_t PULL_CTL     = 0;
        uint8_t NIRQ_MODE    = 0;
    } NIRQ;

    struct
    {
        uint8_t X            = 0;
        uint8_t PULL_CTL     = 0;
        uint8_t SDO_MODE     = 0;
    } SDO;

    struct
    {
        uint8_t X            = 0;
        uint8_t DRV_STRENGTH = 0;
        uint8_t XXXXX        = 0;
    } GEN_CONFIG;
};

struct GPIO_PIN_CFG_REP
{
    struct
    {
        uint8_t GPIO_STATE   = 0;
        uint8_t X            = 0;
        uint8_t GPIO_MODE    = 0;
    } GPIO0;

    struct
    {
        uint8_t GPIO_STATE   = 0;
        uint8_t X            = 0;
        uint8_t GPIO_MODE    = 0;
    } GPIO1;

    struct
    {
        uint8_t GPIO_STATE   = 0;
        uint8_t X            = 0;
        uint8_t GPIO_MODE    = 0;
    } GPIO2;

    struct
    {
        uint8_t GPIO_STATE   = 0;
        uint8_t X            = 0;
        uint8_t GPIO_MODE    = 0;
    } GPIO3;

    struct
    {
        uint8_t NIRQ_STATE   = 0;
        uint8_t X            = 0;
        uint8_t NIRQ_MODE    = 0;
    } NIQR;

    struct
    {
        uint8_t SDO_STATE    = 0;
        uint8_t X            = 0;
        uint8_t SDO_MODE     = 0;
    } SDO;

    struct
    {
        uint8_t X            = 0;
        uint8_t DRV_STRENGTH = 0;
        uint8_t XXXXX        = 0;
    } GEN_CONFIG;
};

uint8_t Command_GPIO_PIN_CFG(GPIO_PIN_CFG_REQ &req, GPIO_PIN_CFG_REP &rep)
{
    const uint8_t CMD_ID       = 0x13;
    const uint8_t BUF_SIZE_REQ = 7;
    const uint8_t BUF_SIZE_REP = 7;

    uint8_t bufReq[BUF_SIZE_REQ];
    uint8_t bufRep[BUF_SIZE_REP];

    // pack request data into buffer
    uint8_t tmpReqByte0 = 0;
    tmpReqByte0 |= (uint8_t)((req.GPIO0.X                 & 0b00000001) << 7);
    tmpReqByte0 |= (uint8_t)((req.GPIO0.PULL_CTL          & 0b00000001) << 6);
    tmpReqByte0 |= (uint8_t)((req.GPIO0.GPIO_MODE         & 0b00111111) << 0);
    bufReq[0] = tmpReqByte0;

    uint8_t tmpReqByte1 = 0;
    tmpReqByte1 |= (uint8_t)((req.GPIO1.X                 & 0b00000001) << 7);
    tmpReqByte1 |= (uint8_t)((req.GPIO1.PULL_CTL          & 0b00000001) << 6);
    tmpReqByte1 |= (uint8_t)((req.GPIO1.GPIO_MODE         & 0b00111111) << 0);
    bufReq[1] = tmpReqByte1;

    uint8_t tmpReqByte2 = 0;
    tmpReqByte2 |= (uint8_t)((req.GPIO2.X                 & 0b00000001) << 7);
    tmpReqByte2 |= (uint8_t)((req.GPIO2.PULL_CTL          & 0b00000001) << 6);
    tmpReqByte2 |= (uint8_t)((req.GPIO2.GPIO_MODE         & 0b00111111) << 0);
    bufReq[2] = tmpReqByte2;

    uint8_t tmpReqByte3 = 0;
    tmpReqByte3 |= (uint8_t)((req.GPIO3.X                 & 0b00000001) << 7);
    tmpReqByte3 |= (uint8_t)((req.GPIO3.PULL_CTL          & 0b00000001) << 6);
    tmpReqByte3 |= (uint8_t)((req.GPIO3.GPIO_MODE         & 0b00111111) << 0);
    bufReq[3] = tmpReqByte3;

    uint8_t tmpReqByte4 = 0;
    tmpReqByte4 |= (uint8_t)((req.NIRQ.X                  & 0b00000001) << 7);
    tmpReqByte4 |= (uint8_t)((req.NIRQ.PULL_CTL           & 0b00000001) << 6);
    tmpReqByte4 |= (uint8_t)((req.NIRQ.NIRQ_MODE          & 0b00111111) << 0);
    bufReq[4] = tmpReqByte4;

    uint8_t tmpReqByte5 = 0;
    tmpReqByte5 |= (uint8_t)((req.SDO.X                   & 0b00000001) << 7);
    tmpReqByte5 |= (uint8_t)((req.SDO.PULL_CTL            & 0b00000001) << 6);
    tmpReqByte5 |= (uint8_t)((req.SDO.SDO_MODE            & 0b00111111) << 0);
    bufReq[5] = tmpReqByte5;

    uint8_t tmpReqByte6 = 0;
    tmpReqByte6 |= (uint8_t)((req.GEN_CONFIG.X            & 0b00000001) << 7);
    tmpReqByte6 |= (uint8_t)((req.GEN_CONFIG.DRV_STRENGTH & 0b00000011) << 5);
    tmpReqByte6 |= (uint8_t)((req.GEN_CONFIG.XXXXX        & 0b00011111) << 0);
    bufReq[6] = tmpReqByte6;

    uint8_t ok = SendAndWaitAndReceive(CMD_ID, bufReq, BUF_SIZE_REQ, bufRep, BUF_SIZE_REP);

    if (ok)
    {
        BufferFieldExtractor bfe(bufRep, BUF_SIZE_REP);

        uint8_t tmpByte0 = bfe.GetUI8();
        rep.GPIO0.GPIO_STATE        = (uint8_t)((tmpByte0 & 0b10000000) >> 7);
        rep.GPIO0.X                 = (uint8_t)((tmpByte0 & 0b01000000) >> 6);
        rep.GPIO0.GPIO_MODE         = (uint8_t)((tmpByte0 & 0b00111111) >> 0);

        uint8_t tmpByte1 = bfe.GetUI8();
        rep.GPIO1.GPIO_STATE        = (uint8_t)((tmpByte1 & 0b10000000) >> 7);
        rep.GPIO1.X                 = (uint8_t)((tmpByte1 & 0b01000000) >> 6);
        rep.GPIO1.GPIO_MODE         = (uint8_t)((tmpByte1 & 0b00111111) >> 0);

        uint8_t tmpByte2 = bfe.GetUI8();
        rep.GPIO2.GPIO_STATE        = (uint8_t)((tmpByte2 & 0b10000000) >> 7);
        rep.GPIO2.X                 = (uint8_t)((tmpByte2 & 0b01000000) >> 6);
        rep.GPIO2.GPIO_MODE         = (uint8_t)((tmpByte2 & 0b00111111) >> 0);

        uint8_t tmpByte3 = bfe.GetUI8();
        rep.GPIO3.GPIO_STATE        = (uint8_t)((tmpByte3 & 0b10000000) >> 7);
        rep.GPIO3.X                 = (uint8_t)((tmpByte3 & 0b01000000) >> 6);
        rep.GPIO3.GPIO_MODE         = (uint8_t)((tmpByte3 & 0b00111111) >> 0);

        uint8_t tmpByte4 = bfe.GetUI8();
        rep.NIQR.NIRQ_STATE         = (uint8_t)((tmpByte4 & 0b10000000) >> 7);
        rep.NIQR.X                  = (uint8_t)((tmpByte4 & 0b01000000) >> 6);
        rep.NIQR.NIRQ_MODE          = (uint8_t)((tmpByte4 & 0b00111111) >> 0);

        uint8_t tmpByte5 = bfe.GetUI8();
        rep.SDO.SDO_STATE           = (uint8_t)((tmpByte5 & 0b10000000) >> 7);
        rep.SDO.X                   = (uint8_t)((tmpByte5 & 0b01000000) >> 6);
        rep.SDO.SDO_MODE            = (uint8_t)((tmpByte5 & 0b00111111) >> 0);

        uint8_t tmpByte6 = bfe.GetUI8();
        rep.GEN_CONFIG.X            = (uint8_t)((tmpByte6 & 0b10000000) >> 7);
        rep.GEN_CONFIG.DRV_STRENGTH = (uint8_t)((tmpByte6 & 0b01100000) >> 5);
        rep.GEN_CONFIG.XXXXX        = (uint8_t)((tmpByte6 & 0b00011111) >> 0);
    }

    return ok;
}

////////////////////////////////////////////////////////////////////
//
// COMMAND REQUEST_DEVICE_STATE (0x33)
//
////////////////////////////////////////////////////////////////////

struct REQUEST_DEVICE_STATE_REP
{
    struct
    {
        uint8_t XXXX            = 0;
        uint8_t MAIN_STATE      = 0;
    } CURR_STATE;

    struct
    {
        uint8_t CURRENT_CHANNEL = 0;
    } CURRENT_CHANNEL;
};

uint8_t Command_REQUEST_DEVICE_STATE(REQUEST_DEVICE_STATE_REP &rep)
{
    const uint8_t CMD_ID       = 0x33;
    const uint8_t BUF_SIZE_REQ = 0;
    const uint8_t BUF_SIZE_REP = 2;

    uint8_t bufReq[BUF_SIZE_REQ];
    uint8_t bufRep[BUF_SIZE_REP];

    uint8_t ok = SendAndWaitAndReceive(CMD_ID, bufReq, BUF_SIZE_REQ, bufRep, BUF_SIZE_REP);

    if (ok)
    {
        BufferFieldExtractor bfe(bufRep, BUF_SIZE_REP);

        uint8_t tmpByte0 = bfe.GetUI8();
        rep.CURR_STATE.XXXX                 = (uint8_t)((tmpByte0 & 0b11110000) >> 4);
        rep.CURR_STATE.MAIN_STATE           = (uint8_t)((tmpByte0 & 0b00001111) >> 0);

        rep.CURRENT_CHANNEL.CURRENT_CHANNEL = bfe.GetUI8();
    }

    return ok;
}

////////////////////////////////////////////////////////////////////
//
// COMMAND SET_PROPERTY (0x11)
//
////////////////////////////////////////////////////////////////////

struct SET_PROPERTY_REQ
{
    struct
    {
        uint8_t GROUP      = 0;
    } GROUP;

    struct
    {
        uint8_t NUM_PROPS  = 0;
    } NUM_PROPS;

    struct
    {
        uint8_t START_PROP = 0;
    } START_PROP;

    struct
    {
        uint8_t DATA0      = 0;
    } DATA0;

    struct
    {
        uint8_t DATA1      = 0;
    } DATA1;

    struct
    {
        uint8_t DATA2      = 0;
    } DATA2;

    struct
    {
        uint8_t DATA3      = 0;
    } DATA3;

    struct
    {
        uint8_t DATA4      = 0;
    } DATA4;

    struct
    {
        uint8_t DATA5      = 0;
    } DATA5;

    struct
    {
        uint8_t DATA6      = 0;
    } DATA6;

    struct
    {
        uint8_t DATA7      = 0;
    } DATA7;

    struct
    {
        uint8_t DATA8      = 0;
    } DATA8;

    struct
    {
        uint8_t DATA9      = 0;
    } DATA9;

    struct
    {
        uint8_t DATA10     = 0;
    } DATA10;

    struct
    {
        uint8_t DATA11     = 0;
    } DATA11;
};

uint8_t Command_SET_PROPERTY(SET_PROPERTY_REQ &req)
{
    const uint8_t CMD_ID       = 0x11;
    const uint8_t BUF_SIZE_REQ = 15;
    const uint8_t BUF_SIZE_REP = 0;

    uint8_t bufReq[BUF_SIZE_REQ];
    uint8_t bufRep[BUF_SIZE_REP];

    // pack request data into buffer
    bufReq[0] = req.GROUP.GROUP;

    bufReq[1] = req.NUM_PROPS.NUM_PROPS;

    bufReq[2] = req.START_PROP.START_PROP;

    bufReq[3] = req.DATA0.DATA0;

    bufReq[4] = req.DATA1.DATA1;

    bufReq[5] = req.DATA2.DATA2;

    bufReq[6] = req.DATA3.DATA3;

    bufReq[7] = req.DATA4.DATA4;

    bufReq[8] = req.DATA5.DATA5;

    bufReq[9] = req.DATA6.DATA6;

    bufReq[10] = req.DATA7.DATA7;

    bufReq[11] = req.DATA8.DATA8;

    bufReq[12] = req.DATA9.DATA9;

    bufReq[13] = req.DATA10.DATA10;

    bufReq[14] = req.DATA11.DATA11;

    uint8_t ok = SendAndWaitAndReceive(CMD_ID, bufReq, BUF_SIZE_REQ, bufRep, BUF_SIZE_REP);

    return ok;
}

////////////////////////////////////////////////////////////////////
//
// COMMAND GET_PROPERTY (0x12)
//
////////////////////////////////////////////////////////////////////

struct GET_PROPERTY_REQ
{
    struct
    {
        uint8_t GROUP      = 0;
    } GROUP;

    struct
    {
        uint8_t NUM_PROPS  = 0;
    } NUM_PROPS;

    struct
    {
        uint8_t START_PROP = 0;
    } START_PROP;
};

struct GET_PROPERTY_REP
{
    struct
    {
        uint8_t DATA0  = 0;
    } DATA0;

    struct
    {
        uint8_t DATA1  = 0;
    } DATA1;

    struct
    {
        uint8_t DATA2  = 0;
    } DATA2;

    struct
    {
        uint8_t DATA3  = 0;
    } DATA3;

    struct
    {
        uint8_t DATA4  = 0;
    } DATA4;

    struct
    {
        uint8_t DATA5  = 0;
    } DATA5;

    struct
    {
        uint8_t DATA6  = 0;
    } DATA6;

    struct
    {
        uint8_t DATA7  = 0;
    } DATA7;

    struct
    {
        uint8_t DATA8  = 0;
    } DATA8;

    struct
    {
        uint8_t DATA9  = 0;
    } DATA9;

    struct
    {
        uint8_t DATA10 = 0;
    } DATA10;

    struct
    {
        uint8_t DATA11 = 0;
    } DATA11;

    struct
    {
        uint8_t DATA12 = 0;
    } DATA12;

    struct
    {
        uint8_t DATA13 = 0;
    } DATA13;

    struct
    {
        uint8_t DATA14 = 0;
    } DATA14;

    struct
    {
        uint8_t DATA15 = 0;
    } DATA15;
};

uint8_t Command_GET_PROPERTY(GET_PROPERTY_REQ &req, GET_PROPERTY_REP &rep)
{
    const uint8_t CMD_ID       = 0x12;
    const uint8_t BUF_SIZE_REQ = 3;
    const uint8_t BUF_SIZE_REP = 16;

    uint8_t bufReq[BUF_SIZE_REQ];
    uint8_t bufRep[BUF_SIZE_REP];

    // pack request data into buffer
    bufReq[0] = req.GROUP.GROUP;

    bufReq[1] = req.NUM_PROPS.NUM_PROPS;

    bufReq[2] = req.START_PROP.START_PROP;

    uint8_t ok = SendAndWaitAndReceive(CMD_ID, bufReq, BUF_SIZE_REQ, bufRep, BUF_SIZE_REP);

    if (ok)
    {
        BufferFieldExtractor bfe(bufRep, BUF_SIZE_REP);

        rep.DATA0.DATA0   = bfe.GetUI8();

        rep.DATA1.DATA1   = bfe.GetUI8();

        rep.DATA2.DATA2   = bfe.GetUI8();

        rep.DATA3.DATA3   = bfe.GetUI8();

        rep.DATA4.DATA4   = bfe.GetUI8();

        rep.DATA5.DATA5   = bfe.GetUI8();

        rep.DATA6.DATA6   = bfe.GetUI8();

        rep.DATA7.DATA7   = bfe.GetUI8();

        rep.DATA8.DATA8   = bfe.GetUI8();

        rep.DATA9.DATA9   = bfe.GetUI8();

        rep.DATA10.DATA10 = bfe.GetUI8();

        rep.DATA11.DATA11 = bfe.GetUI8();

        rep.DATA12.DATA12 = bfe.GetUI8();

        rep.DATA13.DATA13 = bfe.GetUI8();

        rep.DATA14.DATA14 = bfe.GetUI8();

        rep.DATA15.DATA15 = bfe.GetUI8();
    }

    return ok;
}

////////////////////////////////////////////////////////////////////
//
// COMMAND GET_CHIP_STATUS (0x23)
//
////////////////////////////////////////////////////////////////////

struct GET_CHIP_STATUS_REP
{
    struct
    {
        uint8_t CAL_PEND                           = 0;
        uint8_t FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND = 0;
        uint8_t STATE_CHANGE_PEND                  = 0;
        uint8_t CMD_ERROR_PEND                     = 0;
        uint8_t CHIP_READY_PEND                    = 0;
        uint8_t LOW_BATT_PEND                      = 0;
        uint8_t WUT_PEND                           = 0;
    } CHIP_PEND;

    struct
    {
        uint8_t CAL                                = 0;
        uint8_t FIFO_UNDERFLOW_OVERFLOW_ERROR      = 0;
        uint8_t STATE_CHANGE                       = 0;
        uint8_t CMD_ERROR                          = 0;
        uint8_t CHIP_READY                         = 0;
        uint8_t LOW_BATT                           = 0;
        uint8_t WUT                                = 0;
    } CHIP_STATUS;

    struct
    {
        uint8_t CMD_ERR_STATUS                     = 0;
    } CMD_ERR_STATUS;
};

uint8_t Command_GET_CHIP_STATUS(GET_CHIP_STATUS_REP &rep)
{
    const uint8_t CMD_ID       = 0x23;
    const uint8_t BUF_SIZE_REQ = 0;
    const uint8_t BUF_SIZE_REP = 3;

    uint8_t bufReq[BUF_SIZE_REQ];
    uint8_t bufRep[BUF_SIZE_REP];

    uint8_t ok = SendAndWaitAndReceive(CMD_ID, bufReq, BUF_SIZE_REQ, bufRep, BUF_SIZE_REP);

    if (ok)
    {
        BufferFieldExtractor bfe(bufRep, BUF_SIZE_REP);

        uint8_t tmpByte0 = bfe.GetUI8();
        rep.CHIP_PEND.CAL_PEND                           = (uint8_t)((tmpByte0 & 0b11000000) >> 6);
        rep.CHIP_PEND.FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND = (uint8_t)((tmpByte0 & 0b00100000) >> 5);
        rep.CHIP_PEND.STATE_CHANGE_PEND                  = (uint8_t)((tmpByte0 & 0b00010000) >> 4);
        rep.CHIP_PEND.CMD_ERROR_PEND                     = (uint8_t)((tmpByte0 & 0b00001000) >> 3);
        rep.CHIP_PEND.CHIP_READY_PEND                    = (uint8_t)((tmpByte0 & 0b00000100) >> 2);
        rep.CHIP_PEND.LOW_BATT_PEND                      = (uint8_t)((tmpByte0 & 0b00000010) >> 1);
        rep.CHIP_PEND.WUT_PEND                           = (uint8_t)((tmpByte0 & 0b00000001) >> 0);

        uint8_t tmpByte1 = bfe.GetUI8();
        rep.CHIP_STATUS.CAL                              = (uint8_t)((tmpByte1 & 0b11000000) >> 6);
        rep.CHIP_STATUS.FIFO_UNDERFLOW_OVERFLOW_ERROR    = (uint8_t)((tmpByte1 & 0b00100000) >> 5);
        rep.CHIP_STATUS.STATE_CHANGE                     = (uint8_t)((tmpByte1 & 0b00010000) >> 4);
        rep.CHIP_STATUS.CMD_ERROR                        = (uint8_t)((tmpByte1 & 0b00001000) >> 3);
        rep.CHIP_STATUS.CHIP_READY                       = (uint8_t)((tmpByte1 & 0b00000100) >> 2);
        rep.CHIP_STATUS.LOW_BATT                         = (uint8_t)((tmpByte1 & 0b00000010) >> 1);
        rep.CHIP_STATUS.WUT                              = (uint8_t)((tmpByte1 & 0b00000001) >> 0);

        rep.CMD_ERR_STATUS.CMD_ERR_STATUS                = bfe.GetUI8();
    }

    return ok;
}

////////////////////////////////////////////////////////////////////
//
// COMMAND FIFO_INFO (0x15)
//
////////////////////////////////////////////////////////////////////

struct FIFO_INFO_REQ
{
    struct
    {
        uint8_t ZERO = 0;
        uint8_t RX   = 0;
        uint8_t TX   = 0;
    } FIFO;
};

struct FIFO_INFO_REP
{
    struct
    {
        uint8_t RX_FIFO_COUNT = 0;
    } RX_FIFO_COUNT;

    struct
    {
        uint8_t TX_FIFO_SPACE = 0;
    } TX_FIFO_SPACE;
};

uint8_t Command_FIFO_INFO(FIFO_INFO_REQ &req, FIFO_INFO_REP &rep)
{
    const uint8_t CMD_ID       = 0x15;
    const uint8_t BUF_SIZE_REQ = 1;
    const uint8_t BUF_SIZE_REP = 2;

    uint8_t bufReq[BUF_SIZE_REQ];
    uint8_t bufRep[BUF_SIZE_REP];

    // pack request data into buffer
    uint8_t tmpReqByte0 = 0;
    tmpReqByte0 |= (uint8_t)((req.FIFO.ZERO & 0b00111111) << 2);
    tmpReqByte0 |= (uint8_t)((req.FIFO.RX   & 0b00000001) << 1);
    tmpReqByte0 |= (uint8_t)((req.FIFO.TX   & 0b00000001) << 0);
    bufReq[0] = tmpReqByte0;

    uint8_t ok = SendAndWaitAndReceive(CMD_ID, bufReq, BUF_SIZE_REQ, bufRep, BUF_SIZE_REP);

    if (ok)
    {
        BufferFieldExtractor bfe(bufRep, BUF_SIZE_REP);

        rep.RX_FIFO_COUNT.RX_FIFO_COUNT = bfe.GetUI8();

        rep.TX_FIFO_SPACE.TX_FIFO_SPACE = bfe.GetUI8();
    }

    return ok;
}

////////////////////////////////////////////////////////////////////
//
// PROPERTY MODEM_CLKGEN_BAND (0x20 0x51)
//
////////////////////////////////////////////////////////////////////

struct MODEM_CLKGEN_BAND_PROP
{
    struct
    {
        uint8_t XXX                  = 0;
        uint8_t FORCE_SY_RECAL       = 0;
        uint8_t SY_SEL               = 0;
        uint8_t BAND                 = 0;
    } BYTE0;
};

uint8_t SetProperty(MODEM_CLKGEN_BAND_PROP &prop)
{
    const uint8_t PROP_GROUP = 0x20;
    const uint8_t PROP_IDX   = 0x51;
    const uint8_t BUF_SIZE   = 1;

    uint8_t buf[BUF_SIZE];

    // pack request data into buffer
    uint8_t tmpReqByte0 = 0;
    tmpReqByte0 |= (uint8_t)((prop.BYTE0.XXX            & 0b00000111) << 5);
    tmpReqByte0 |= (uint8_t)((prop.BYTE0.FORCE_SY_RECAL & 0b00000001) << 4);
    tmpReqByte0 |= (uint8_t)((prop.BYTE0.SY_SEL         & 0b00000001) << 3);
    tmpReqByte0 |= (uint8_t)((prop.BYTE0.BAND           & 0b00000111) << 0);
    buf[0] = tmpReqByte0;

    uint8_t retVal = SetProperty(PROP_GROUP, PROP_IDX, *buf);

    return retVal;
}

////////////////////////////////////////////////////////////////////
//
// PROPERTY FREQ_CONTROL_INTE (0x40 0x00)
//
////////////////////////////////////////////////////////////////////

struct FREQ_CONTROL_INTE_PROP
{
    struct
    {
        uint8_t X          = 0;
        uint8_t INTE       = 0;
    } BYTE0;
};

uint8_t SetProperty(FREQ_CONTROL_INTE_PROP &prop)
{
    const uint8_t PROP_GROUP = 0x40;
    const uint8_t PROP_IDX   = 0x00;
    const uint8_t BUF_SIZE   = 1;

    uint8_t buf[BUF_SIZE];

    // pack request data into buffer
    uint8_t tmpReqByte0 = 0;
    tmpReqByte0 |= (uint8_t)((prop.BYTE0.X    & 0b00000001) << 7);
    tmpReqByte0 |= (uint8_t)((prop.BYTE0.INTE & 0b01111111) << 0);
    buf[0] = tmpReqByte0;

    uint8_t retVal = SetProperty(PROP_GROUP, PROP_IDX, *buf);

    return retVal;
}

////////////////////////////////////////////////////////////////////
//
// PROPERTY MODEM_MOD_TYPE (0x20 0x00)
//
////////////////////////////////////////////////////////////////////

struct MODEM_MOD_TYPE_PROP
{
    struct
    {
        uint8_t TX_DIRECT_MODE_TYPE       = 0;
        uint8_t TX_DIRECT_MODE_GPIO       = 0;
        uint8_t MOD_SOURCE                = 0;
        uint8_t MOD_TYPE                  = 0;
    } BYTE0;
};

uint8_t SetProperty(MODEM_MOD_TYPE_PROP &prop)
{
    const uint8_t PROP_GROUP = 0x20;
    const uint8_t PROP_IDX   = 0x00;
    const uint8_t BUF_SIZE   = 1;

    uint8_t buf[BUF_SIZE];

    // pack request data into buffer
    uint8_t tmpReqByte0 = 0;
    tmpReqByte0 |= (uint8_t)((prop.BYTE0.TX_DIRECT_MODE_TYPE & 0b00000001) << 7);
    tmpReqByte0 |= (uint8_t)((prop.BYTE0.TX_DIRECT_MODE_GPIO & 0b00000011) << 5);
    tmpReqByte0 |= (uint8_t)((prop.BYTE0.MOD_SOURCE          & 0b00000011) << 3);
    tmpReqByte0 |= (uint8_t)((prop.BYTE0.MOD_TYPE            & 0b00000111) << 0);
    buf[0] = tmpReqByte0;

    uint8_t retVal = SetProperty(PROP_GROUP, PROP_IDX, *buf);

    return retVal;
}

////////////////////////////////////////////////////////////////////
//
// PROPERTY MODEM_FREQ_DEV (0x20 0x0A)
//
////////////////////////////////////////////////////////////////////

struct MODEM_FREQ_DEV_PROP
{
    struct
    {
        uint8_t X             = 0;
        uint8_t FREQDEV       = 0;
    } BYTE0;

    struct
    {
        uint8_t FREQDEV       = 0;
    } BYTE1;

    struct
    {
        uint8_t FREQDEV       = 0;
    } BYTE2;
};

uint8_t SetProperty(MODEM_FREQ_DEV_PROP &prop)
{
    const uint8_t PROP_GROUP = 0x20;
    const uint8_t PROP_IDX   = 0x0A;
    const uint8_t BUF_SIZE   = 3;

    uint8_t buf[BUF_SIZE];

    // pack request data into buffer
    uint8_t tmpReqByte0 = 0;
    tmpReqByte0 |= (uint8_t)((prop.BYTE0.X       & 0b01111111) << 1);
    tmpReqByte0 |= (uint8_t)((prop.BYTE0.FREQDEV & 0b00000001) << 0);
    buf[0] = tmpReqByte0;

    buf[1] = prop.BYTE1.FREQDEV;

    buf[2] = prop.BYTE2.FREQDEV;

    uint8_t retVal = SetProperty(PROP_GROUP, PROP_IDX, *buf);

    return retVal;
}

