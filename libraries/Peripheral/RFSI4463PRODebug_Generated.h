////////////////////////////////////////////////////////////////////
//
// PART_INFO (0x01)
//
////////////////////////////////////////////////////////////////////

void Print(RFSI4463PRO::PART_INFO_REP &val)
{
    Serial.println("PART_INFO_REP");

    Serial.print("CHIPREV.CHIPREV  : ");
    Serial.print(val.CHIPREV.CHIPREV);
    Serial.print(" (0x");
    Serial.print(val.CHIPREV.CHIPREV, HEX);
    Serial.print(")");
    Serial.println();
    Serial.print("PART.PART        : ");
    Serial.print(val.PART.PART);
    Serial.print(" (0x");
    Serial.print(val.PART.PART, HEX);
    Serial.print(")");
    Serial.println();
    Serial.print("PBUILD.PBUILD    : ");
    Serial.print(val.PBUILD.PBUILD);
    Serial.print(" (0x");
    Serial.print(val.PBUILD.PBUILD, HEX);
    Serial.print(")");
    Serial.println();
    Serial.print("ID.ID            : ");
    Serial.print(val.ID.ID);
    Serial.print(" (0x");
    Serial.print(val.ID.ID, HEX);
    Serial.print(")");
    Serial.println();
    Serial.print("CUSTOMER.CUSTOMER: ");
    Serial.print(val.CUSTOMER.CUSTOMER);
    Serial.print(" (0x");
    Serial.print(val.CUSTOMER.CUSTOMER, HEX);
    Serial.print(")");
    Serial.println();
    Serial.print("ROMID.ROMID      : ");
    Serial.print(val.ROMID.ROMID);
    Serial.print(" (0x");
    Serial.print(val.ROMID.ROMID, HEX);
    Serial.print(")");
    Serial.println();
}

////////////////////////////////////////////////////////////////////
//
// FUNC_INFO (0x10)
//
////////////////////////////////////////////////////////////////////

void Print(RFSI4463PRO::FUNC_INFO_REP &val)
{
    Serial.println("FUNC_INFO_REP");

    Serial.print("REVEXT.REVEXT      : ");
    Serial.print(val.REVEXT.REVEXT);
    Serial.print(" (0x");
    Serial.print(val.REVEXT.REVEXT, HEX);
    Serial.print(")");
    Serial.println();
    Serial.print("REVBRANCH.REVBRANCH: ");
    Serial.print(val.REVBRANCH.REVBRANCH);
    Serial.print(" (0x");
    Serial.print(val.REVBRANCH.REVBRANCH, HEX);
    Serial.print(")");
    Serial.println();
    Serial.print("REVINT.REVINT      : ");
    Serial.print(val.REVINT.REVINT);
    Serial.print(" (0x");
    Serial.print(val.REVINT.REVINT, HEX);
    Serial.print(")");
    Serial.println();
    Serial.print("PATCH.PATCH        : ");
    Serial.print(val.PATCH.PATCH);
    Serial.print(" (0x");
    Serial.print(val.PATCH.PATCH, HEX);
    Serial.print(")");
    Serial.println();
    Serial.print("FUNC.FUNC          : ");
    Serial.print(val.FUNC.FUNC);
    Serial.print(" (0x");
    Serial.print(val.FUNC.FUNC, HEX);
    Serial.print(")");
    Serial.println();
}

////////////////////////////////////////////////////////////////////
//
// GET_CHIP_STATUS (0x23)
//
////////////////////////////////////////////////////////////////////

void Print(RFSI4463PRO::GET_CHIP_STATUS_REP &val)
{
    Serial.println("GET_CHIP_STATUS_REP");

    Serial.print("CHIP_PEND.CAL_PEND                          : ");
    Serial.print(val.CHIP_PEND.CAL_PEND);
    Serial.print(" (0x");
    Serial.print(val.CHIP_PEND.CAL_PEND, HEX);
    Serial.print(")");
    Serial.println();
    Serial.print("CHIP_PEND.FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND: ");
    Serial.print(val.CHIP_PEND.FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND);
    Serial.print(" (0x");
    Serial.print(val.CHIP_PEND.FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND, HEX);
    Serial.print(")");
    Serial.println();
    Serial.print("CHIP_PEND.STATE_CHANGE_PEND                 : ");
    Serial.print(val.CHIP_PEND.STATE_CHANGE_PEND);
    Serial.print(" (0x");
    Serial.print(val.CHIP_PEND.STATE_CHANGE_PEND, HEX);
    Serial.print(")");
    Serial.println();
    Serial.print("CHIP_PEND.CMD_ERROR_PEND                    : ");
    Serial.print(val.CHIP_PEND.CMD_ERROR_PEND);
    Serial.print(" (0x");
    Serial.print(val.CHIP_PEND.CMD_ERROR_PEND, HEX);
    Serial.print(")");
    Serial.println();
    Serial.print("CHIP_PEND.CHIP_READY_PEND                   : ");
    Serial.print(val.CHIP_PEND.CHIP_READY_PEND);
    Serial.print(" (0x");
    Serial.print(val.CHIP_PEND.CHIP_READY_PEND, HEX);
    Serial.print(")");
    Serial.println();
    Serial.print("CHIP_PEND.LOW_BATT_PEND                     : ");
    Serial.print(val.CHIP_PEND.LOW_BATT_PEND);
    Serial.print(" (0x");
    Serial.print(val.CHIP_PEND.LOW_BATT_PEND, HEX);
    Serial.print(")");
    Serial.println();
    Serial.print("CHIP_PEND.WUT_PEND                          : ");
    Serial.print(val.CHIP_PEND.WUT_PEND);
    Serial.print(" (0x");
    Serial.print(val.CHIP_PEND.WUT_PEND, HEX);
    Serial.print(")");
    Serial.println();
    Serial.print("CHIP_STATUS.CAL                             : ");
    Serial.print(val.CHIP_STATUS.CAL);
    Serial.print(" (0x");
    Serial.print(val.CHIP_STATUS.CAL, HEX);
    Serial.print(")");
    Serial.println();
    Serial.print("CHIP_STATUS.FIFO_UNDERFLOW_OVERFLOW_ERROR   : ");
    Serial.print(val.CHIP_STATUS.FIFO_UNDERFLOW_OVERFLOW_ERROR);
    Serial.print(" (0x");
    Serial.print(val.CHIP_STATUS.FIFO_UNDERFLOW_OVERFLOW_ERROR, HEX);
    Serial.print(")");
    Serial.println();
    Serial.print("CHIP_STATUS.STATE_CHANGE                    : ");
    Serial.print(val.CHIP_STATUS.STATE_CHANGE);
    Serial.print(" (0x");
    Serial.print(val.CHIP_STATUS.STATE_CHANGE, HEX);
    Serial.print(")");
    Serial.println();
    Serial.print("CHIP_STATUS.CMD_ERROR                       : ");
    Serial.print(val.CHIP_STATUS.CMD_ERROR);
    Serial.print(" (0x");
    Serial.print(val.CHIP_STATUS.CMD_ERROR, HEX);
    Serial.print(")");
    Serial.println();
    Serial.print("CHIP_STATUS.CHIP_READY                      : ");
    Serial.print(val.CHIP_STATUS.CHIP_READY);
    Serial.print(" (0x");
    Serial.print(val.CHIP_STATUS.CHIP_READY, HEX);
    Serial.print(")");
    Serial.println();
    Serial.print("CHIP_STATUS.LOW_BATT                        : ");
    Serial.print(val.CHIP_STATUS.LOW_BATT);
    Serial.print(" (0x");
    Serial.print(val.CHIP_STATUS.LOW_BATT, HEX);
    Serial.print(")");
    Serial.println();
    Serial.print("CHIP_STATUS.WUT                             : ");
    Serial.print(val.CHIP_STATUS.WUT);
    Serial.print(" (0x");
    Serial.print(val.CHIP_STATUS.WUT, HEX);
    Serial.print(")");
    Serial.println();
    Serial.print("CMD_ERR_STATUS.CMD_ERR_STATUS               : ");
    Serial.print(val.CMD_ERR_STATUS.CMD_ERR_STATUS);
    Serial.print(" (0x");
    Serial.print(val.CMD_ERR_STATUS.CMD_ERR_STATUS, HEX);
    Serial.print(")");
    Serial.println();
}

////////////////////////////////////////////////////////////////////
//
// FIFO_INFO (0x15)
//
////////////////////////////////////////////////////////////////////

void Print(RFSI4463PRO::FIFO_INFO_REP &val)
{
    Serial.println("FIFO_INFO_REP");

    Serial.print("RX_FIFO_COUNT.RX_FIFO_COUNT: ");
    Serial.print(val.RX_FIFO_COUNT.RX_FIFO_COUNT);
    Serial.print(" (0x");
    Serial.print(val.RX_FIFO_COUNT.RX_FIFO_COUNT, HEX);
    Serial.print(")");
    Serial.println();
    Serial.print("TX_FIFO_SPACE.TX_FIFO_SPACE: ");
    Serial.print(val.TX_FIFO_SPACE.TX_FIFO_SPACE);
    Serial.print(" (0x");
    Serial.print(val.TX_FIFO_SPACE.TX_FIFO_SPACE, HEX);
    Serial.print(")");
    Serial.println();
}

////////////////////////////////////////////////////////////////////
//
// REQUEST_DEVICE_STATE (0x33)
//
////////////////////////////////////////////////////////////////////

void Print(RFSI4463PRO::REQUEST_DEVICE_STATE_REP &val)
{
    Serial.println("REQUEST_DEVICE_STATE_REP");

    Serial.print("CURR_STATE.ZERO                : ");
    Serial.print(val.CURR_STATE.ZERO);
    Serial.print(" (0x");
    Serial.print(val.CURR_STATE.ZERO, HEX);
    Serial.print(")");
    Serial.println();
    Serial.print("CURR_STATE.MAIN_STATE          : ");
    Serial.print(val.CURR_STATE.MAIN_STATE);
    Serial.print(" (0x");
    Serial.print(val.CURR_STATE.MAIN_STATE, HEX);
    Serial.print(")");
    Serial.println();
    Serial.print("CURRENT_CHANNEL.CURRENT_CHANNEL: ");
    Serial.print(val.CURRENT_CHANNEL.CURRENT_CHANNEL);
    Serial.print(" (0x");
    Serial.print(val.CURRENT_CHANNEL.CURRENT_CHANNEL, HEX);
    Serial.print(")");
    Serial.println();
}

