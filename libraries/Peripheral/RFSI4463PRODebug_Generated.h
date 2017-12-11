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

