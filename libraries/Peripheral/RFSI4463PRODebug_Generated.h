////////////////////////////////////////////////////////////////////
//
// PART_INFO (0x01)
//
////////////////////////////////////////////////////////////////////

void Print(RFSI4463PRO::PART_INFO_REP &val)
{
    Serial.println(F("PART_INFO_REP"));

    Serial.print(F("CHIPREV.CHIPREV   : "));
    Serial.print(val.CHIPREV.CHIPREV);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIPREV.CHIPREV, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("PART.PART         : "));
    Serial.print(val.PART.PART);
    Serial.print(F(" (0x"));
    Serial.print(val.PART.PART, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("PBUILD.PBUILD     : "));
    Serial.print(val.PBUILD.PBUILD);
    Serial.print(F(" (0x"));
    Serial.print(val.PBUILD.PBUILD, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("ID.ID             : "));
    Serial.print(val.ID.ID);
    Serial.print(F(" (0x"));
    Serial.print(val.ID.ID, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CUSTOMER.CUSTOMER : "));
    Serial.print(val.CUSTOMER.CUSTOMER);
    Serial.print(F(" (0x"));
    Serial.print(val.CUSTOMER.CUSTOMER, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("ROMID.ROMID       : "));
    Serial.print(val.ROMID.ROMID);
    Serial.print(F(" (0x"));
    Serial.print(val.ROMID.ROMID, HEX);
    Serial.print(F(")"));
    Serial.println();
}

////////////////////////////////////////////////////////////////////
//
// FUNC_INFO (0x10)
//
////////////////////////////////////////////////////////////////////

void Print(RFSI4463PRO::FUNC_INFO_REP &val)
{
    Serial.println(F("FUNC_INFO_REP"));

    Serial.print(F("REVEXT.REVEXT       : "));
    Serial.print(val.REVEXT.REVEXT);
    Serial.print(F(" (0x"));
    Serial.print(val.REVEXT.REVEXT, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("REVBRANCH.REVBRANCH : "));
    Serial.print(val.REVBRANCH.REVBRANCH);
    Serial.print(F(" (0x"));
    Serial.print(val.REVBRANCH.REVBRANCH, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("REVINT.REVINT       : "));
    Serial.print(val.REVINT.REVINT);
    Serial.print(F(" (0x"));
    Serial.print(val.REVINT.REVINT, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("PATCH.PATCH         : "));
    Serial.print(val.PATCH.PATCH);
    Serial.print(F(" (0x"));
    Serial.print(val.PATCH.PATCH, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("FUNC.FUNC           : "));
    Serial.print(val.FUNC.FUNC);
    Serial.print(F(" (0x"));
    Serial.print(val.FUNC.FUNC, HEX);
    Serial.print(F(")"));
    Serial.println();
}

////////////////////////////////////////////////////////////////////
//
// GET_INT_STATUS (0x20)
//
////////////////////////////////////////////////////////////////////

void Print(RFSI4463PRO::GET_INT_STATUS_REP &val)
{
    Serial.println(F("GET_INT_STATUS_REP"));

    Serial.print(F("INT_PEND.XXXXX                               : "));
    Serial.print(val.INT_PEND.XXXXX);
    Serial.print(F(" (0x"));
    Serial.print(val.INT_PEND.XXXXX, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("INT_PEND.CHIP_INT_STATUS_PEND                : "));
    Serial.print(val.INT_PEND.CHIP_INT_STATUS_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.INT_PEND.CHIP_INT_STATUS_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("INT_PEND.MODEM_INT_STATUS_PEND               : "));
    Serial.print(val.INT_PEND.MODEM_INT_STATUS_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.INT_PEND.MODEM_INT_STATUS_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("INT_PEND.PH_INT_STATUS_PEND                  : "));
    Serial.print(val.INT_PEND.PH_INT_STATUS_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.INT_PEND.PH_INT_STATUS_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("INT_STATUS.XXXXX                             : "));
    Serial.print(val.INT_STATUS.XXXXX);
    Serial.print(F(" (0x"));
    Serial.print(val.INT_STATUS.XXXXX, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("INT_STATUS.CHIP_INT_STATUS                   : "));
    Serial.print(val.INT_STATUS.CHIP_INT_STATUS);
    Serial.print(F(" (0x"));
    Serial.print(val.INT_STATUS.CHIP_INT_STATUS, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("INT_STATUS.MODEM_INT_STATUS                  : "));
    Serial.print(val.INT_STATUS.MODEM_INT_STATUS);
    Serial.print(F(" (0x"));
    Serial.print(val.INT_STATUS.MODEM_INT_STATUS, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("INT_STATUS.PH_INT_STATUS                     : "));
    Serial.print(val.INT_STATUS.PH_INT_STATUS);
    Serial.print(F(" (0x"));
    Serial.print(val.INT_STATUS.PH_INT_STATUS, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("PH_PEND.FILTER_MATCH_PEND                    : "));
    Serial.print(val.PH_PEND.FILTER_MATCH_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.PH_PEND.FILTER_MATCH_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("PH_PEND.FILTER_MISS_PEND                     : "));
    Serial.print(val.PH_PEND.FILTER_MISS_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.PH_PEND.FILTER_MISS_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("PH_PEND.PACKET_SENT_PEND                     : "));
    Serial.print(val.PH_PEND.PACKET_SENT_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.PH_PEND.PACKET_SENT_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("PH_PEND.PACKET_RX_PEND                       : "));
    Serial.print(val.PH_PEND.PACKET_RX_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.PH_PEND.PACKET_RX_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("PH_PEND.CRC_ERROR_PEND                       : "));
    Serial.print(val.PH_PEND.CRC_ERROR_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.PH_PEND.CRC_ERROR_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("PH_PEND.X                                    : "));
    Serial.print(val.PH_PEND.X);
    Serial.print(F(" (0x"));
    Serial.print(val.PH_PEND.X, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("PH_PEND.TX_FIFO_ALMOST_EMPTY_PEND            : "));
    Serial.print(val.PH_PEND.TX_FIFO_ALMOST_EMPTY_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.PH_PEND.TX_FIFO_ALMOST_EMPTY_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("PH_PEND.RX_FIFO_ALMOST_FULL_PEND             : "));
    Serial.print(val.PH_PEND.RX_FIFO_ALMOST_FULL_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.PH_PEND.RX_FIFO_ALMOST_FULL_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("PH_STATUS.FILTER_MATCH                       : "));
    Serial.print(val.PH_STATUS.FILTER_MATCH);
    Serial.print(F(" (0x"));
    Serial.print(val.PH_STATUS.FILTER_MATCH, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("PH_STATUS.FILTER_MISS                        : "));
    Serial.print(val.PH_STATUS.FILTER_MISS);
    Serial.print(F(" (0x"));
    Serial.print(val.PH_STATUS.FILTER_MISS, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("PH_STATUS.PACKET_SENT                        : "));
    Serial.print(val.PH_STATUS.PACKET_SENT);
    Serial.print(F(" (0x"));
    Serial.print(val.PH_STATUS.PACKET_SENT, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("PH_STATUS.PACKET_RX                          : "));
    Serial.print(val.PH_STATUS.PACKET_RX);
    Serial.print(F(" (0x"));
    Serial.print(val.PH_STATUS.PACKET_RX, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("PH_STATUS.CRC_ERROR                          : "));
    Serial.print(val.PH_STATUS.CRC_ERROR);
    Serial.print(F(" (0x"));
    Serial.print(val.PH_STATUS.CRC_ERROR, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("PH_STATUS.X                                  : "));
    Serial.print(val.PH_STATUS.X);
    Serial.print(F(" (0x"));
    Serial.print(val.PH_STATUS.X, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("PH_STATUS.TX_FIFO_ALMOST_EMPTY               : "));
    Serial.print(val.PH_STATUS.TX_FIFO_ALMOST_EMPTY);
    Serial.print(F(" (0x"));
    Serial.print(val.PH_STATUS.TX_FIFO_ALMOST_EMPTY, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("PH_STATUS.RX_FIFO_ALMOST_FULL                : "));
    Serial.print(val.PH_STATUS.RX_FIFO_ALMOST_FULL);
    Serial.print(F(" (0x"));
    Serial.print(val.PH_STATUS.RX_FIFO_ALMOST_FULL, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("MODEM_PEND.XX                                : "));
    Serial.print(val.MODEM_PEND.XX);
    Serial.print(F(" (0x"));
    Serial.print(val.MODEM_PEND.XX, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("MODEM_PEND.INVALID_SYNC_PEND                 : "));
    Serial.print(val.MODEM_PEND.INVALID_SYNC_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.MODEM_PEND.INVALID_SYNC_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("MODEM_PEND.RSSI_JUMP_PEND                    : "));
    Serial.print(val.MODEM_PEND.RSSI_JUMP_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.MODEM_PEND.RSSI_JUMP_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("MODEM_PEND.RSSI_PEND                         : "));
    Serial.print(val.MODEM_PEND.RSSI_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.MODEM_PEND.RSSI_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("MODEM_PEND.INVALID_PREAMBLE_PEND             : "));
    Serial.print(val.MODEM_PEND.INVALID_PREAMBLE_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.MODEM_PEND.INVALID_PREAMBLE_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("MODEM_PEND.PREAMBLE_DETECT_PEND              : "));
    Serial.print(val.MODEM_PEND.PREAMBLE_DETECT_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.MODEM_PEND.PREAMBLE_DETECT_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("MODEM_PEND.SYNC_DETECT_PEND                  : "));
    Serial.print(val.MODEM_PEND.SYNC_DETECT_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.MODEM_PEND.SYNC_DETECT_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("MODEM_STATUS.XX                              : "));
    Serial.print(val.MODEM_STATUS.XX);
    Serial.print(F(" (0x"));
    Serial.print(val.MODEM_STATUS.XX, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("MODEM_STATUS.INVALID_SYNC                    : "));
    Serial.print(val.MODEM_STATUS.INVALID_SYNC);
    Serial.print(F(" (0x"));
    Serial.print(val.MODEM_STATUS.INVALID_SYNC, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("MODEM_STATUS.RSSI_JUMP                       : "));
    Serial.print(val.MODEM_STATUS.RSSI_JUMP);
    Serial.print(F(" (0x"));
    Serial.print(val.MODEM_STATUS.RSSI_JUMP, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("MODEM_STATUS.RSSI                            : "));
    Serial.print(val.MODEM_STATUS.RSSI);
    Serial.print(F(" (0x"));
    Serial.print(val.MODEM_STATUS.RSSI, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("MODEM_STATUS.INVALID_PREAMBLE                : "));
    Serial.print(val.MODEM_STATUS.INVALID_PREAMBLE);
    Serial.print(F(" (0x"));
    Serial.print(val.MODEM_STATUS.INVALID_PREAMBLE, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("MODEM_STATUS.PREAMBLE_DETECT                 : "));
    Serial.print(val.MODEM_STATUS.PREAMBLE_DETECT);
    Serial.print(F(" (0x"));
    Serial.print(val.MODEM_STATUS.PREAMBLE_DETECT, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("MODEM_STATUS.SYNC_DETECT                     : "));
    Serial.print(val.MODEM_STATUS.SYNC_DETECT);
    Serial.print(F(" (0x"));
    Serial.print(val.MODEM_STATUS.SYNC_DETECT, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CHIP_PEND.CAL_PEND                           : "));
    Serial.print(val.CHIP_PEND.CAL_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_PEND.CAL_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CHIP_PEND.FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND : "));
    Serial.print(val.CHIP_PEND.FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_PEND.FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CHIP_PEND.STATE_CHANGE_PEND                  : "));
    Serial.print(val.CHIP_PEND.STATE_CHANGE_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_PEND.STATE_CHANGE_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CHIP_PEND.CMD_ERROR_PEND                     : "));
    Serial.print(val.CHIP_PEND.CMD_ERROR_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_PEND.CMD_ERROR_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CHIP_PEND.CHIP_READY_PEND                    : "));
    Serial.print(val.CHIP_PEND.CHIP_READY_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_PEND.CHIP_READY_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CHIP_PEND.LOW_BATT_PEND                      : "));
    Serial.print(val.CHIP_PEND.LOW_BATT_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_PEND.LOW_BATT_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CHIP_PEND.WUT_PEND                           : "));
    Serial.print(val.CHIP_PEND.WUT_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_PEND.WUT_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CHIP_STATUS.CAL                              : "));
    Serial.print(val.CHIP_STATUS.CAL);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_STATUS.CAL, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CHIP_STATUS.FIFO_UNDERFLOW_OVERFLOW_ERROR    : "));
    Serial.print(val.CHIP_STATUS.FIFO_UNDERFLOW_OVERFLOW_ERROR);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_STATUS.FIFO_UNDERFLOW_OVERFLOW_ERROR, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CHIP_STATUS.STATE_CHANGE                     : "));
    Serial.print(val.CHIP_STATUS.STATE_CHANGE);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_STATUS.STATE_CHANGE, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CHIP_STATUS.CMD_ERROR                        : "));
    Serial.print(val.CHIP_STATUS.CMD_ERROR);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_STATUS.CMD_ERROR, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CHIP_STATUS.CHIP_READY                       : "));
    Serial.print(val.CHIP_STATUS.CHIP_READY);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_STATUS.CHIP_READY, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CHIP_STATUS.LOW_BATT                         : "));
    Serial.print(val.CHIP_STATUS.LOW_BATT);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_STATUS.LOW_BATT, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CHIP_STATUS.WUT                              : "));
    Serial.print(val.CHIP_STATUS.WUT);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_STATUS.WUT, HEX);
    Serial.print(F(")"));
    Serial.println();
}

////////////////////////////////////////////////////////////////////
//
// GET_PROPERTY (0x12)
//
////////////////////////////////////////////////////////////////////

void Print(RFSI4463PRO::GET_PROPERTY_REP &val)
{
    Serial.println(F("GET_PROPERTY_REP"));

    Serial.print(F("DATA0.DATA0   : "));
    Serial.print(val.DATA0.DATA0);
    Serial.print(F(" (0x"));
    Serial.print(val.DATA0.DATA0, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("DATA1.DATA1   : "));
    Serial.print(val.DATA1.DATA1);
    Serial.print(F(" (0x"));
    Serial.print(val.DATA1.DATA1, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("DATA2.DATA2   : "));
    Serial.print(val.DATA2.DATA2);
    Serial.print(F(" (0x"));
    Serial.print(val.DATA2.DATA2, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("DATA3.DATA3   : "));
    Serial.print(val.DATA3.DATA3);
    Serial.print(F(" (0x"));
    Serial.print(val.DATA3.DATA3, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("DATA4.DATA4   : "));
    Serial.print(val.DATA4.DATA4);
    Serial.print(F(" (0x"));
    Serial.print(val.DATA4.DATA4, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("DATA5.DATA5   : "));
    Serial.print(val.DATA5.DATA5);
    Serial.print(F(" (0x"));
    Serial.print(val.DATA5.DATA5, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("DATA6.DATA6   : "));
    Serial.print(val.DATA6.DATA6);
    Serial.print(F(" (0x"));
    Serial.print(val.DATA6.DATA6, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("DATA7.DATA7   : "));
    Serial.print(val.DATA7.DATA7);
    Serial.print(F(" (0x"));
    Serial.print(val.DATA7.DATA7, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("DATA8.DATA8   : "));
    Serial.print(val.DATA8.DATA8);
    Serial.print(F(" (0x"));
    Serial.print(val.DATA8.DATA8, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("DATA9.DATA9   : "));
    Serial.print(val.DATA9.DATA9);
    Serial.print(F(" (0x"));
    Serial.print(val.DATA9.DATA9, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("DATA10.DATA10 : "));
    Serial.print(val.DATA10.DATA10);
    Serial.print(F(" (0x"));
    Serial.print(val.DATA10.DATA10, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("DATA11.DATA11 : "));
    Serial.print(val.DATA11.DATA11);
    Serial.print(F(" (0x"));
    Serial.print(val.DATA11.DATA11, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("DATA12.DATA12 : "));
    Serial.print(val.DATA12.DATA12);
    Serial.print(F(" (0x"));
    Serial.print(val.DATA12.DATA12, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("DATA13.DATA13 : "));
    Serial.print(val.DATA13.DATA13);
    Serial.print(F(" (0x"));
    Serial.print(val.DATA13.DATA13, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("DATA14.DATA14 : "));
    Serial.print(val.DATA14.DATA14);
    Serial.print(F(" (0x"));
    Serial.print(val.DATA14.DATA14, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("DATA15.DATA15 : "));
    Serial.print(val.DATA15.DATA15);
    Serial.print(F(" (0x"));
    Serial.print(val.DATA15.DATA15, HEX);
    Serial.print(F(")"));
    Serial.println();
}

////////////////////////////////////////////////////////////////////
//
// GET_CHIP_STATUS (0x23)
//
////////////////////////////////////////////////////////////////////

void Print(RFSI4463PRO::GET_CHIP_STATUS_REP &val)
{
    Serial.println(F("GET_CHIP_STATUS_REP"));

    Serial.print(F("CHIP_PEND.CAL_PEND                           : "));
    Serial.print(val.CHIP_PEND.CAL_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_PEND.CAL_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CHIP_PEND.FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND : "));
    Serial.print(val.CHIP_PEND.FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_PEND.FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CHIP_PEND.STATE_CHANGE_PEND                  : "));
    Serial.print(val.CHIP_PEND.STATE_CHANGE_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_PEND.STATE_CHANGE_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CHIP_PEND.CMD_ERROR_PEND                     : "));
    Serial.print(val.CHIP_PEND.CMD_ERROR_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_PEND.CMD_ERROR_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CHIP_PEND.CHIP_READY_PEND                    : "));
    Serial.print(val.CHIP_PEND.CHIP_READY_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_PEND.CHIP_READY_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CHIP_PEND.LOW_BATT_PEND                      : "));
    Serial.print(val.CHIP_PEND.LOW_BATT_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_PEND.LOW_BATT_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CHIP_PEND.WUT_PEND                           : "));
    Serial.print(val.CHIP_PEND.WUT_PEND);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_PEND.WUT_PEND, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CHIP_STATUS.CAL                              : "));
    Serial.print(val.CHIP_STATUS.CAL);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_STATUS.CAL, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CHIP_STATUS.FIFO_UNDERFLOW_OVERFLOW_ERROR    : "));
    Serial.print(val.CHIP_STATUS.FIFO_UNDERFLOW_OVERFLOW_ERROR);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_STATUS.FIFO_UNDERFLOW_OVERFLOW_ERROR, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CHIP_STATUS.STATE_CHANGE                     : "));
    Serial.print(val.CHIP_STATUS.STATE_CHANGE);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_STATUS.STATE_CHANGE, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CHIP_STATUS.CMD_ERROR                        : "));
    Serial.print(val.CHIP_STATUS.CMD_ERROR);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_STATUS.CMD_ERROR, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CHIP_STATUS.CHIP_READY                       : "));
    Serial.print(val.CHIP_STATUS.CHIP_READY);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_STATUS.CHIP_READY, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CHIP_STATUS.LOW_BATT                         : "));
    Serial.print(val.CHIP_STATUS.LOW_BATT);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_STATUS.LOW_BATT, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CHIP_STATUS.WUT                              : "));
    Serial.print(val.CHIP_STATUS.WUT);
    Serial.print(F(" (0x"));
    Serial.print(val.CHIP_STATUS.WUT, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CMD_ERR_STATUS.CMD_ERR_STATUS                : "));
    Serial.print(val.CMD_ERR_STATUS.CMD_ERR_STATUS);
    Serial.print(F(" (0x"));
    Serial.print(val.CMD_ERR_STATUS.CMD_ERR_STATUS, HEX);
    Serial.print(F(")"));
    Serial.println();
}

////////////////////////////////////////////////////////////////////
//
// FIFO_INFO (0x15)
//
////////////////////////////////////////////////////////////////////

void Print(RFSI4463PRO::FIFO_INFO_REP &val)
{
    Serial.println(F("FIFO_INFO_REP"));

    Serial.print(F("RX_FIFO_COUNT.RX_FIFO_COUNT : "));
    Serial.print(val.RX_FIFO_COUNT.RX_FIFO_COUNT);
    Serial.print(F(" (0x"));
    Serial.print(val.RX_FIFO_COUNT.RX_FIFO_COUNT, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("TX_FIFO_SPACE.TX_FIFO_SPACE : "));
    Serial.print(val.TX_FIFO_SPACE.TX_FIFO_SPACE);
    Serial.print(F(" (0x"));
    Serial.print(val.TX_FIFO_SPACE.TX_FIFO_SPACE, HEX);
    Serial.print(F(")"));
    Serial.println();
}

////////////////////////////////////////////////////////////////////
//
// REQUEST_DEVICE_STATE (0x33)
//
////////////////////////////////////////////////////////////////////

void Print(RFSI4463PRO::REQUEST_DEVICE_STATE_REP &val)
{
    Serial.println(F("REQUEST_DEVICE_STATE_REP"));

    Serial.print(F("CURR_STATE.ZERO                 : "));
    Serial.print(val.CURR_STATE.ZERO);
    Serial.print(F(" (0x"));
    Serial.print(val.CURR_STATE.ZERO, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CURR_STATE.MAIN_STATE           : "));
    Serial.print(val.CURR_STATE.MAIN_STATE);
    Serial.print(F(" (0x"));
    Serial.print(val.CURR_STATE.MAIN_STATE, HEX);
    Serial.print(F(")"));
    Serial.println();
    Serial.print(F("CURRENT_CHANNEL.CURRENT_CHANNEL : "));
    Serial.print(val.CURRENT_CHANNEL.CURRENT_CHANNEL);
    Serial.print(F(" (0x"));
    Serial.print(val.CURRENT_CHANNEL.CURRENT_CHANNEL, HEX);
    Serial.print(F(")"));
    Serial.println();
}

