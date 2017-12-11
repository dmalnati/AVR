////////////////////////////////////////////////////////////////////
//
// PART_INFO (0x01)
//
////////////////////////////////////////////////////////////////////

struct PART_INFO_REP
{
    struct
    {
        uint8_t CHIPREV;
    } CHIPREV;

    struct
    {
        uint16_t PART;
    } PART;

    struct
    {
        uint8_t PBUILD;
    } PBUILD;

    struct
    {
        uint16_t ID;
    } ID;

    struct
    {
        uint8_t CUSTOMER;
    } CUSTOMER;

    struct
    {
        uint8_t ROMID;
    } ROMID;
};

uint8_t Command_PART_INFO(PART_INFO_REP &retVal)
{
    const uint8_t CMD_ID   = 0x01;
    const uint8_t BUF_SIZE = 8;

    uint8_t buf[BUF_SIZE];

    uint8_t ok = SendAndWaitAndReceive(CMD_ID, buf, BUF_SIZE);

    if (ok)
    {
        BufferFieldExtractor bfe(buf, BUF_SIZE);

        retVal.CHIPREV.CHIPREV   = bfe.GetUI8();

        retVal.PART.PART         = bfe.GetUI16NTOHS();

        retVal.PBUILD.PBUILD     = bfe.GetUI8();

        retVal.ID.ID             = bfe.GetUI16NTOHS();

        retVal.CUSTOMER.CUSTOMER = bfe.GetUI8();

        retVal.ROMID.ROMID       = bfe.GetUI8();
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
        uint8_t REVEXT;
    } REVEXT;

    struct
    {
        uint8_t REVBRANCH;
    } REVBRANCH;

    struct
    {
        uint8_t REVINT;
    } REVINT;

    struct
    {
        uint16_t PATCH;
    } PATCH;

    struct
    {
        uint8_t FUNC;
    } FUNC;
};

uint8_t Command_FUNC_INFO(FUNC_INFO_REP &retVal)
{
    const uint8_t CMD_ID   = 0x10;
    const uint8_t BUF_SIZE = 6;

    uint8_t buf[BUF_SIZE];

    uint8_t ok = SendAndWaitAndReceive(CMD_ID, buf, BUF_SIZE);

    if (ok)
    {
        BufferFieldExtractor bfe(buf, BUF_SIZE);

        retVal.REVEXT.REVEXT       = bfe.GetUI8();

        retVal.REVBRANCH.REVBRANCH = bfe.GetUI8();

        retVal.REVINT.REVINT       = bfe.GetUI8();

        retVal.PATCH.PATCH         = bfe.GetUI16NTOHS();

        retVal.FUNC.FUNC           = bfe.GetUI8();
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
        uint8_t CAL_PEND;
        uint8_t FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND;
        uint8_t STATE_CHANGE_PEND;
        uint8_t CMD_ERROR_PEND;
        uint8_t CHIP_READY_PEND;
        uint8_t LOW_BATT_PEND;
        uint8_t WUT_PEND;
    } CHIP_PEND;

    struct
    {
        uint8_t CAL;
        uint8_t FIFO_UNDERFLOW_OVERFLOW_ERROR;
        uint8_t STATE_CHANGE;
        uint8_t CMD_ERROR;
        uint8_t CHIP_READY;
        uint8_t LOW_BATT;
        uint8_t WUT;
    } CHIP_STATUS;

    struct
    {
        uint8_t CMD_ERR_STATUS;
    } CMD_ERR_STATUS;
};

uint8_t Command_GET_CHIP_STATUS(GET_CHIP_STATUS_REP &retVal)
{
    const uint8_t CMD_ID   = 0x23;
    const uint8_t BUF_SIZE = 3;

    uint8_t buf[BUF_SIZE];

    uint8_t ok = SendAndWaitAndReceive(CMD_ID, buf, BUF_SIZE);

    if (ok)
    {
        BufferFieldExtractor bfe(buf, BUF_SIZE);

        uint8_t tmpByte0 = bfe.GetUI8();
        retVal.CHIP_PEND.CAL_PEND                           = (uint8_t)((tmpByte0 & 0b11000000) >> 6);
        retVal.CHIP_PEND.FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND = (uint8_t)((tmpByte0 & 0b00100000) >> 5);
        retVal.CHIP_PEND.STATE_CHANGE_PEND                  = (uint8_t)((tmpByte0 & 0b00010000) >> 4);
        retVal.CHIP_PEND.CMD_ERROR_PEND                     = (uint8_t)((tmpByte0 & 0b00001000) >> 3);
        retVal.CHIP_PEND.CHIP_READY_PEND                    = (uint8_t)((tmpByte0 & 0b00000100) >> 2);
        retVal.CHIP_PEND.LOW_BATT_PEND                      = (uint8_t)((tmpByte0 & 0b00000010) >> 1);
        retVal.CHIP_PEND.WUT_PEND                           = (uint8_t)((tmpByte0 & 0b00000001) >> 0);

        uint8_t tmpByte1 = bfe.GetUI8();
        retVal.CHIP_STATUS.CAL                              = (uint8_t)((tmpByte1 & 0b11000000) >> 6);
        retVal.CHIP_STATUS.FIFO_UNDERFLOW_OVERFLOW_ERROR    = (uint8_t)((tmpByte1 & 0b00100000) >> 5);
        retVal.CHIP_STATUS.STATE_CHANGE                     = (uint8_t)((tmpByte1 & 0b00010000) >> 4);
        retVal.CHIP_STATUS.CMD_ERROR                        = (uint8_t)((tmpByte1 & 0b00001000) >> 3);
        retVal.CHIP_STATUS.CHIP_READY                       = (uint8_t)((tmpByte1 & 0b00000100) >> 2);
        retVal.CHIP_STATUS.LOW_BATT                         = (uint8_t)((tmpByte1 & 0b00000010) >> 1);
        retVal.CHIP_STATUS.WUT                              = (uint8_t)((tmpByte1 & 0b00000001) >> 0);

        retVal.CMD_ERR_STATUS.CMD_ERR_STATUS                = bfe.GetUI8();
    }

    return ok;
}

