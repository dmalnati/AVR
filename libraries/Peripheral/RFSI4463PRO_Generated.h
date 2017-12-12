////////////////////////////////////////////////////////////////////
//
// PART_INFO (0x01)
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
// FUNC_INFO (0x10)
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
// GET_CHIP_STATUS (0x23)
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
// FIFO_INFO (0x15)
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
// REQUEST_DEVICE_STATE (0x33)
//
////////////////////////////////////////////////////////////////////

struct REQUEST_DEVICE_STATE_REP
{
    struct
    {
        uint8_t ZERO            = 0;
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
        rep.CURR_STATE.ZERO                 = (uint8_t)((tmpByte0 & 0b11110000) >> 4);
        rep.CURR_STATE.MAIN_STATE           = (uint8_t)((tmpByte0 & 0b00001111) >> 0);

        rep.CURRENT_CHANNEL.CURRENT_CHANNEL = bfe.GetUI8();
    }

    return ok;
}

