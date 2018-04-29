////////////////////////////////////////////////////////////////////
//
// COMMAND PART_INFO (0x01)
//
////////////////////////////////////////////////////////////////////

void Print(RFSI4463PRO::PART_INFO_REP &val)
{
    @fix@Serial.println(F("PART_INFO_REP"));

    @fix@Serial.print(F("CHIPREV.CHIPREV   : "));
    @fix@Serial.print(val.CHIPREV.CHIPREV);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIPREV.CHIPREV, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("PART.PART         : "));
    @fix@Serial.print(val.PART.PART);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.PART.PART, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("PBUILD.PBUILD     : "));
    @fix@Serial.print(val.PBUILD.PBUILD);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.PBUILD.PBUILD, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("ID.ID             : "));
    @fix@Serial.print(val.ID.ID);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.ID.ID, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CUSTOMER.CUSTOMER : "));
    @fix@Serial.print(val.CUSTOMER.CUSTOMER);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CUSTOMER.CUSTOMER, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("ROMID.ROMID       : "));
    @fix@Serial.print(val.ROMID.ROMID);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.ROMID.ROMID, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
}

////////////////////////////////////////////////////////////////////
//
// COMMAND FUNC_INFO (0x10)
//
////////////////////////////////////////////////////////////////////

void Print(RFSI4463PRO::FUNC_INFO_REP &val)
{
    @fix@Serial.println(F("FUNC_INFO_REP"));

    @fix@Serial.print(F("REVEXT.REVEXT       : "));
    @fix@Serial.print(val.REVEXT.REVEXT);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.REVEXT.REVEXT, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("REVBRANCH.REVBRANCH : "));
    @fix@Serial.print(val.REVBRANCH.REVBRANCH);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.REVBRANCH.REVBRANCH, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("REVINT.REVINT       : "));
    @fix@Serial.print(val.REVINT.REVINT);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.REVINT.REVINT, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("PATCH.PATCH         : "));
    @fix@Serial.print(val.PATCH.PATCH);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.PATCH.PATCH, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("FUNC.FUNC           : "));
    @fix@Serial.print(val.FUNC.FUNC);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.FUNC.FUNC, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
}

////////////////////////////////////////////////////////////////////
//
// COMMAND GET_INT_STATUS (0x20)
//
////////////////////////////////////////////////////////////////////

void Print(RFSI4463PRO::GET_INT_STATUS_REP &val)
{
    @fix@Serial.println(F("GET_INT_STATUS_REP"));

    @fix@Serial.print(F("INT_PEND.XXXXX                               : "));
    @fix@Serial.print(val.INT_PEND.XXXXX);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.INT_PEND.XXXXX, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("INT_PEND.CHIP_INT_STATUS_PEND                : "));
    @fix@Serial.print(val.INT_PEND.CHIP_INT_STATUS_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.INT_PEND.CHIP_INT_STATUS_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("INT_PEND.MODEM_INT_STATUS_PEND               : "));
    @fix@Serial.print(val.INT_PEND.MODEM_INT_STATUS_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.INT_PEND.MODEM_INT_STATUS_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("INT_PEND.PH_INT_STATUS_PEND                  : "));
    @fix@Serial.print(val.INT_PEND.PH_INT_STATUS_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.INT_PEND.PH_INT_STATUS_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("INT_STATUS.XXXXX                             : "));
    @fix@Serial.print(val.INT_STATUS.XXXXX);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.INT_STATUS.XXXXX, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("INT_STATUS.CHIP_INT_STATUS                   : "));
    @fix@Serial.print(val.INT_STATUS.CHIP_INT_STATUS);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.INT_STATUS.CHIP_INT_STATUS, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("INT_STATUS.MODEM_INT_STATUS                  : "));
    @fix@Serial.print(val.INT_STATUS.MODEM_INT_STATUS);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.INT_STATUS.MODEM_INT_STATUS, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("INT_STATUS.PH_INT_STATUS                     : "));
    @fix@Serial.print(val.INT_STATUS.PH_INT_STATUS);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.INT_STATUS.PH_INT_STATUS, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("PH_PEND.FILTER_MATCH_PEND                    : "));
    @fix@Serial.print(val.PH_PEND.FILTER_MATCH_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.PH_PEND.FILTER_MATCH_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("PH_PEND.FILTER_MISS_PEND                     : "));
    @fix@Serial.print(val.PH_PEND.FILTER_MISS_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.PH_PEND.FILTER_MISS_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("PH_PEND.PACKET_SENT_PEND                     : "));
    @fix@Serial.print(val.PH_PEND.PACKET_SENT_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.PH_PEND.PACKET_SENT_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("PH_PEND.PACKET_RX_PEND                       : "));
    @fix@Serial.print(val.PH_PEND.PACKET_RX_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.PH_PEND.PACKET_RX_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("PH_PEND.CRC_ERROR_PEND                       : "));
    @fix@Serial.print(val.PH_PEND.CRC_ERROR_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.PH_PEND.CRC_ERROR_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("PH_PEND.X                                    : "));
    @fix@Serial.print(val.PH_PEND.X);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.PH_PEND.X, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("PH_PEND.TX_FIFO_ALMOST_EMPTY_PEND            : "));
    @fix@Serial.print(val.PH_PEND.TX_FIFO_ALMOST_EMPTY_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.PH_PEND.TX_FIFO_ALMOST_EMPTY_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("PH_PEND.RX_FIFO_ALMOST_FULL_PEND             : "));
    @fix@Serial.print(val.PH_PEND.RX_FIFO_ALMOST_FULL_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.PH_PEND.RX_FIFO_ALMOST_FULL_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("PH_STATUS.FILTER_MATCH                       : "));
    @fix@Serial.print(val.PH_STATUS.FILTER_MATCH);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.PH_STATUS.FILTER_MATCH, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("PH_STATUS.FILTER_MISS                        : "));
    @fix@Serial.print(val.PH_STATUS.FILTER_MISS);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.PH_STATUS.FILTER_MISS, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("PH_STATUS.PACKET_SENT                        : "));
    @fix@Serial.print(val.PH_STATUS.PACKET_SENT);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.PH_STATUS.PACKET_SENT, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("PH_STATUS.PACKET_RX                          : "));
    @fix@Serial.print(val.PH_STATUS.PACKET_RX);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.PH_STATUS.PACKET_RX, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("PH_STATUS.CRC_ERROR                          : "));
    @fix@Serial.print(val.PH_STATUS.CRC_ERROR);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.PH_STATUS.CRC_ERROR, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("PH_STATUS.X                                  : "));
    @fix@Serial.print(val.PH_STATUS.X);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.PH_STATUS.X, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("PH_STATUS.TX_FIFO_ALMOST_EMPTY               : "));
    @fix@Serial.print(val.PH_STATUS.TX_FIFO_ALMOST_EMPTY);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.PH_STATUS.TX_FIFO_ALMOST_EMPTY, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("PH_STATUS.RX_FIFO_ALMOST_FULL                : "));
    @fix@Serial.print(val.PH_STATUS.RX_FIFO_ALMOST_FULL);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.PH_STATUS.RX_FIFO_ALMOST_FULL, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("MODEM_PEND.XX                                : "));
    @fix@Serial.print(val.MODEM_PEND.XX);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.MODEM_PEND.XX, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("MODEM_PEND.INVALID_SYNC_PEND                 : "));
    @fix@Serial.print(val.MODEM_PEND.INVALID_SYNC_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.MODEM_PEND.INVALID_SYNC_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("MODEM_PEND.RSSI_JUMP_PEND                    : "));
    @fix@Serial.print(val.MODEM_PEND.RSSI_JUMP_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.MODEM_PEND.RSSI_JUMP_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("MODEM_PEND.RSSI_PEND                         : "));
    @fix@Serial.print(val.MODEM_PEND.RSSI_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.MODEM_PEND.RSSI_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("MODEM_PEND.INVALID_PREAMBLE_PEND             : "));
    @fix@Serial.print(val.MODEM_PEND.INVALID_PREAMBLE_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.MODEM_PEND.INVALID_PREAMBLE_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("MODEM_PEND.PREAMBLE_DETECT_PEND              : "));
    @fix@Serial.print(val.MODEM_PEND.PREAMBLE_DETECT_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.MODEM_PEND.PREAMBLE_DETECT_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("MODEM_PEND.SYNC_DETECT_PEND                  : "));
    @fix@Serial.print(val.MODEM_PEND.SYNC_DETECT_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.MODEM_PEND.SYNC_DETECT_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("MODEM_STATUS.XX                              : "));
    @fix@Serial.print(val.MODEM_STATUS.XX);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.MODEM_STATUS.XX, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("MODEM_STATUS.INVALID_SYNC                    : "));
    @fix@Serial.print(val.MODEM_STATUS.INVALID_SYNC);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.MODEM_STATUS.INVALID_SYNC, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("MODEM_STATUS.RSSI_JUMP                       : "));
    @fix@Serial.print(val.MODEM_STATUS.RSSI_JUMP);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.MODEM_STATUS.RSSI_JUMP, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("MODEM_STATUS.RSSI                            : "));
    @fix@Serial.print(val.MODEM_STATUS.RSSI);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.MODEM_STATUS.RSSI, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("MODEM_STATUS.INVALID_PREAMBLE                : "));
    @fix@Serial.print(val.MODEM_STATUS.INVALID_PREAMBLE);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.MODEM_STATUS.INVALID_PREAMBLE, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("MODEM_STATUS.PREAMBLE_DETECT                 : "));
    @fix@Serial.print(val.MODEM_STATUS.PREAMBLE_DETECT);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.MODEM_STATUS.PREAMBLE_DETECT, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("MODEM_STATUS.SYNC_DETECT                     : "));
    @fix@Serial.print(val.MODEM_STATUS.SYNC_DETECT);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.MODEM_STATUS.SYNC_DETECT, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CHIP_PEND.CAL_PEND                           : "));
    @fix@Serial.print(val.CHIP_PEND.CAL_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_PEND.CAL_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CHIP_PEND.FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND : "));
    @fix@Serial.print(val.CHIP_PEND.FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_PEND.FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CHIP_PEND.STATE_CHANGE_PEND                  : "));
    @fix@Serial.print(val.CHIP_PEND.STATE_CHANGE_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_PEND.STATE_CHANGE_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CHIP_PEND.CMD_ERROR_PEND                     : "));
    @fix@Serial.print(val.CHIP_PEND.CMD_ERROR_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_PEND.CMD_ERROR_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CHIP_PEND.CHIP_READY_PEND                    : "));
    @fix@Serial.print(val.CHIP_PEND.CHIP_READY_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_PEND.CHIP_READY_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CHIP_PEND.LOW_BATT_PEND                      : "));
    @fix@Serial.print(val.CHIP_PEND.LOW_BATT_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_PEND.LOW_BATT_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CHIP_PEND.WUT_PEND                           : "));
    @fix@Serial.print(val.CHIP_PEND.WUT_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_PEND.WUT_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CHIP_STATUS.CAL                              : "));
    @fix@Serial.print(val.CHIP_STATUS.CAL);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_STATUS.CAL, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CHIP_STATUS.FIFO_UNDERFLOW_OVERFLOW_ERROR    : "));
    @fix@Serial.print(val.CHIP_STATUS.FIFO_UNDERFLOW_OVERFLOW_ERROR);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_STATUS.FIFO_UNDERFLOW_OVERFLOW_ERROR, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CHIP_STATUS.STATE_CHANGE                     : "));
    @fix@Serial.print(val.CHIP_STATUS.STATE_CHANGE);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_STATUS.STATE_CHANGE, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CHIP_STATUS.CMD_ERROR                        : "));
    @fix@Serial.print(val.CHIP_STATUS.CMD_ERROR);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_STATUS.CMD_ERROR, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CHIP_STATUS.CHIP_READY                       : "));
    @fix@Serial.print(val.CHIP_STATUS.CHIP_READY);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_STATUS.CHIP_READY, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CHIP_STATUS.LOW_BATT                         : "));
    @fix@Serial.print(val.CHIP_STATUS.LOW_BATT);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_STATUS.LOW_BATT, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CHIP_STATUS.WUT                              : "));
    @fix@Serial.print(val.CHIP_STATUS.WUT);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_STATUS.WUT, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
}

////////////////////////////////////////////////////////////////////
//
// COMMAND GPIO_PIN_CFG (0x13)
//
////////////////////////////////////////////////////////////////////

void Print(RFSI4463PRO::GPIO_PIN_CFG_REP &val)
{
    @fix@Serial.println(F("GPIO_PIN_CFG_REP"));

    @fix@Serial.print(F("GPIO0.GPIO_STATE        : "));
    @fix@Serial.print(val.GPIO0.GPIO_STATE);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.GPIO0.GPIO_STATE, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("GPIO0.X                 : "));
    @fix@Serial.print(val.GPIO0.X);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.GPIO0.X, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("GPIO0.GPIO_MODE         : "));
    @fix@Serial.print(val.GPIO0.GPIO_MODE);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.GPIO0.GPIO_MODE, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("GPIO1.GPIO_STATE        : "));
    @fix@Serial.print(val.GPIO1.GPIO_STATE);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.GPIO1.GPIO_STATE, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("GPIO1.X                 : "));
    @fix@Serial.print(val.GPIO1.X);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.GPIO1.X, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("GPIO1.GPIO_MODE         : "));
    @fix@Serial.print(val.GPIO1.GPIO_MODE);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.GPIO1.GPIO_MODE, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("GPIO2.GPIO_STATE        : "));
    @fix@Serial.print(val.GPIO2.GPIO_STATE);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.GPIO2.GPIO_STATE, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("GPIO2.X                 : "));
    @fix@Serial.print(val.GPIO2.X);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.GPIO2.X, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("GPIO2.GPIO_MODE         : "));
    @fix@Serial.print(val.GPIO2.GPIO_MODE);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.GPIO2.GPIO_MODE, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("GPIO3.GPIO_STATE        : "));
    @fix@Serial.print(val.GPIO3.GPIO_STATE);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.GPIO3.GPIO_STATE, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("GPIO3.X                 : "));
    @fix@Serial.print(val.GPIO3.X);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.GPIO3.X, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("GPIO3.GPIO_MODE         : "));
    @fix@Serial.print(val.GPIO3.GPIO_MODE);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.GPIO3.GPIO_MODE, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("NIQR.NIRQ_STATE         : "));
    @fix@Serial.print(val.NIQR.NIRQ_STATE);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.NIQR.NIRQ_STATE, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("NIQR.X                  : "));
    @fix@Serial.print(val.NIQR.X);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.NIQR.X, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("NIQR.NIRQ_MODE          : "));
    @fix@Serial.print(val.NIQR.NIRQ_MODE);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.NIQR.NIRQ_MODE, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("SDO.SDO_STATE           : "));
    @fix@Serial.print(val.SDO.SDO_STATE);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.SDO.SDO_STATE, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("SDO.X                   : "));
    @fix@Serial.print(val.SDO.X);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.SDO.X, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("SDO.SDO_MODE            : "));
    @fix@Serial.print(val.SDO.SDO_MODE);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.SDO.SDO_MODE, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("GEN_CONFIG.X            : "));
    @fix@Serial.print(val.GEN_CONFIG.X);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.GEN_CONFIG.X, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("GEN_CONFIG.DRV_STRENGTH : "));
    @fix@Serial.print(val.GEN_CONFIG.DRV_STRENGTH);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.GEN_CONFIG.DRV_STRENGTH, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("GEN_CONFIG.XXXXX        : "));
    @fix@Serial.print(val.GEN_CONFIG.XXXXX);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.GEN_CONFIG.XXXXX, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
}

////////////////////////////////////////////////////////////////////
//
// COMMAND REQUEST_DEVICE_STATE (0x33)
//
////////////////////////////////////////////////////////////////////

void Print(RFSI4463PRO::REQUEST_DEVICE_STATE_REP &val)
{
    @fix@Serial.println(F("REQUEST_DEVICE_STATE_REP"));

    @fix@Serial.print(F("CURR_STATE.XXXX                 : "));
    @fix@Serial.print(val.CURR_STATE.XXXX);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CURR_STATE.XXXX, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CURR_STATE.MAIN_STATE           : "));
    @fix@Serial.print(val.CURR_STATE.MAIN_STATE);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CURR_STATE.MAIN_STATE, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CURRENT_CHANNEL.CURRENT_CHANNEL : "));
    @fix@Serial.print(val.CURRENT_CHANNEL.CURRENT_CHANNEL);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CURRENT_CHANNEL.CURRENT_CHANNEL, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
}

////////////////////////////////////////////////////////////////////
//
// COMMAND GET_PROPERTY (0x12)
//
////////////////////////////////////////////////////////////////////

void Print(RFSI4463PRO::GET_PROPERTY_REP &val)
{
    @fix@Serial.println(F("GET_PROPERTY_REP"));

    @fix@Serial.print(F("DATA0.DATA0   : "));
    @fix@Serial.print(val.DATA0.DATA0);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.DATA0.DATA0, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("DATA1.DATA1   : "));
    @fix@Serial.print(val.DATA1.DATA1);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.DATA1.DATA1, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("DATA2.DATA2   : "));
    @fix@Serial.print(val.DATA2.DATA2);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.DATA2.DATA2, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("DATA3.DATA3   : "));
    @fix@Serial.print(val.DATA3.DATA3);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.DATA3.DATA3, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("DATA4.DATA4   : "));
    @fix@Serial.print(val.DATA4.DATA4);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.DATA4.DATA4, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("DATA5.DATA5   : "));
    @fix@Serial.print(val.DATA5.DATA5);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.DATA5.DATA5, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("DATA6.DATA6   : "));
    @fix@Serial.print(val.DATA6.DATA6);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.DATA6.DATA6, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("DATA7.DATA7   : "));
    @fix@Serial.print(val.DATA7.DATA7);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.DATA7.DATA7, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("DATA8.DATA8   : "));
    @fix@Serial.print(val.DATA8.DATA8);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.DATA8.DATA8, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("DATA9.DATA9   : "));
    @fix@Serial.print(val.DATA9.DATA9);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.DATA9.DATA9, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("DATA10.DATA10 : "));
    @fix@Serial.print(val.DATA10.DATA10);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.DATA10.DATA10, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("DATA11.DATA11 : "));
    @fix@Serial.print(val.DATA11.DATA11);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.DATA11.DATA11, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("DATA12.DATA12 : "));
    @fix@Serial.print(val.DATA12.DATA12);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.DATA12.DATA12, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("DATA13.DATA13 : "));
    @fix@Serial.print(val.DATA13.DATA13);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.DATA13.DATA13, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("DATA14.DATA14 : "));
    @fix@Serial.print(val.DATA14.DATA14);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.DATA14.DATA14, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("DATA15.DATA15 : "));
    @fix@Serial.print(val.DATA15.DATA15);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.DATA15.DATA15, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
}

////////////////////////////////////////////////////////////////////
//
// COMMAND GET_CHIP_STATUS (0x23)
//
////////////////////////////////////////////////////////////////////

void Print(RFSI4463PRO::GET_CHIP_STATUS_REP &val)
{
    @fix@Serial.println(F("GET_CHIP_STATUS_REP"));

    @fix@Serial.print(F("CHIP_PEND.CAL_PEND                           : "));
    @fix@Serial.print(val.CHIP_PEND.CAL_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_PEND.CAL_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CHIP_PEND.FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND : "));
    @fix@Serial.print(val.CHIP_PEND.FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_PEND.FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CHIP_PEND.STATE_CHANGE_PEND                  : "));
    @fix@Serial.print(val.CHIP_PEND.STATE_CHANGE_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_PEND.STATE_CHANGE_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CHIP_PEND.CMD_ERROR_PEND                     : "));
    @fix@Serial.print(val.CHIP_PEND.CMD_ERROR_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_PEND.CMD_ERROR_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CHIP_PEND.CHIP_READY_PEND                    : "));
    @fix@Serial.print(val.CHIP_PEND.CHIP_READY_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_PEND.CHIP_READY_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CHIP_PEND.LOW_BATT_PEND                      : "));
    @fix@Serial.print(val.CHIP_PEND.LOW_BATT_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_PEND.LOW_BATT_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CHIP_PEND.WUT_PEND                           : "));
    @fix@Serial.print(val.CHIP_PEND.WUT_PEND);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_PEND.WUT_PEND, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CHIP_STATUS.CAL                              : "));
    @fix@Serial.print(val.CHIP_STATUS.CAL);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_STATUS.CAL, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CHIP_STATUS.FIFO_UNDERFLOW_OVERFLOW_ERROR    : "));
    @fix@Serial.print(val.CHIP_STATUS.FIFO_UNDERFLOW_OVERFLOW_ERROR);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_STATUS.FIFO_UNDERFLOW_OVERFLOW_ERROR, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CHIP_STATUS.STATE_CHANGE                     : "));
    @fix@Serial.print(val.CHIP_STATUS.STATE_CHANGE);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_STATUS.STATE_CHANGE, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CHIP_STATUS.CMD_ERROR                        : "));
    @fix@Serial.print(val.CHIP_STATUS.CMD_ERROR);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_STATUS.CMD_ERROR, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CHIP_STATUS.CHIP_READY                       : "));
    @fix@Serial.print(val.CHIP_STATUS.CHIP_READY);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_STATUS.CHIP_READY, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CHIP_STATUS.LOW_BATT                         : "));
    @fix@Serial.print(val.CHIP_STATUS.LOW_BATT);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_STATUS.LOW_BATT, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CHIP_STATUS.WUT                              : "));
    @fix@Serial.print(val.CHIP_STATUS.WUT);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CHIP_STATUS.WUT, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("CMD_ERR_STATUS.CMD_ERR_STATUS                : "));
    @fix@Serial.print(val.CMD_ERR_STATUS.CMD_ERR_STATUS);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.CMD_ERR_STATUS.CMD_ERR_STATUS, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
}

////////////////////////////////////////////////////////////////////
//
// COMMAND FIFO_INFO (0x15)
//
////////////////////////////////////////////////////////////////////

void Print(RFSI4463PRO::FIFO_INFO_REP &val)
{
    @fix@Serial.println(F("FIFO_INFO_REP"));

    @fix@Serial.print(F("RX_FIFO_COUNT.RX_FIFO_COUNT : "));
    @fix@Serial.print(val.RX_FIFO_COUNT.RX_FIFO_COUNT);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.RX_FIFO_COUNT.RX_FIFO_COUNT, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
    @fix@Serial.print(F("TX_FIFO_SPACE.TX_FIFO_SPACE : "));
    @fix@Serial.print(val.TX_FIFO_SPACE.TX_FIFO_SPACE);
    @fix@Serial.print(F(" (0x"));
    @fix@Serial.print(val.TX_FIFO_SPACE.TX_FIFO_SPACE, HEX);
    @fix@Serial.print(F(")"));
    @fix@Serial.println();
}

