// RH_RF24.h
// Author: Mike McCauley (mikem@airspayce.com)
// Copyright (C) 2014 Mike McCauley
// $Id: RH_RF24.h,v 1.18 2017/07/25 05:26:50 mikem Exp $
//
// Supports RF24/RF26 and RFM24/RFM26 modules in FIFO mode
// also Si4464/63/62/61/60-A1
// Si4063 is the same but Tx only
//
// Per http://www.hoperf.cn/upload/rf/RFM24.pdf
// and http://www.hoperf.cn/upload/rf/RFM26.pdf
// Sigh: the HopeRF documentation is utter rubbish: full of errors and incomplete. The Si446x docs are better:
// http://www.silabs.com/Support%20Documents/TechnicalDocs/Si4464-63-61-60.pdf
// http://www.silabs.com/Support%20Documents/TechnicalDocs/AN626.pdf
// http://www.silabs.com/Support%20Documents/TechnicalDocs/AN627.pdf
// http://www.silabs.com/Support%20Documents/TechnicalDocs/AN647.pdf
// http://www.silabs.com/Support%20Documents/TechnicalDocs/AN633.pdf
// http://www.silabs.com/Support%20Documents/TechnicalDocs/AN736.pdf
// http://nicerf.com/manage/upfile/indexbanner/635231050196868750.pdf    (API description)
// http://www.silabs.com/Support%20Documents/Software/Si446x%20RX_HOP%20PLL%20Calculator.xlsx
#ifndef RH_RF24_mod_h
#define RH_RF24_mod_h

// Use one of the pre-built radio configuration files
// You can use other WDS generated sample configs accorinding to your needs
// or generate a custom one with WDS and include it here
// See RFSI4463PROPacket/README for file name encoding standard
//#include "RFSI4463PROPacket_Configs/radio_config_Si4464_27_434_2GFSK_5_10.h"
#include "RFSI4463PROPacket_Configs/radio_config_Si4464_30_434_2GFSK_5_10.h"
//#include "RFSI4463PROPacket_Configs/radio_config_Si4464_30_434_2GFSK_10_20.h"
//#include "RFSI4463PROPacket_Configs/radio_config_Si4464_30_915_2GFSK_5_10.h"
//#include "RFSI4463PROPacket_Configs/radio_config_Si4464_30_915_2GFSK_10_20.h"


// This configuration data is defined in radio_config_Si4460.h 
// which was generated with the Silicon Labs WDS program
PROGMEM const uint8_t RF24_CONFIGURATION_DATA[] = RADIO_CONFIGURATION_DATA_ARRAY;


/*
 * Changes I'm making
 * - break support for synchronous ISR-based functionality
 *   - main thread code only
 *   - support pin change interrupts instead of port interrupts
 * - support my core libraries
 * - take in real pin numbers, not arduino pin numbers
 *   - but convert to arduino internally
 * - Log not Serial
 * - Make efficient use of memory
 * - support single-packet flow, no multi-packet data, no streaming
 *   - basically, single packet all-or-nothing data
 * 
 * Basically
 * - external interfaces work the way I am used to (async)
 * - internal resources can make use of core libs to work better
 * 
 */


#include <stdint.h>

#include "IOVec.h"

#include "SPI.h"

#include "Function.h"
#include "PAL.h"
#include "Log.h"
#include "TimedEventHandler.h"
#include "InterruptEventHandler.h"
#include "DurationAuditor.h"


// represent the actual physical FIFO size limit on the chip
#define MODULE_FIFO_SIZE 64

// account for the 1 byte of data we need to send, outbound.
// since we want to send/receive a single packet containing all, we have to
// consider both sending and receiving.
//
// Sending -- we have configured the chip to have a 2 field system:
// - field 1 - size of arbitrary payload (1 byte)
// - field 2 - arbitrary payload (variable)
//
// Therefore, for sending, a max payload of 63 bytes is possible, because 1 byte
// of the chip FIFO needs to get used for the field 1.
//
// Receiving -- we do not receive the 1 byte payload size upon receive, despite
// coming from a sender like us.
// Therefore the max payload could in theory by 64 bytes.
// However, we want a standard send/receive maximum, and so define both to be
// the same.
//
// Pick a size which is 63 or less
#define MAX_PAYLOAD_LEN   32


// Max number of times we will try to read CTS from the radio
#define RH_RF24_CTS_RETRIES 2500

// RF24/RF26 API commands from table 10
// also Si446X API DESCRIPTIONS table 1
#define RH_RF24_CMD_NOP                        0x00
#define RH_RF24_CMD_PART_INFO                  0x01
#define RH_RF24_CMD_POWER_UP                   0x02
#define RH_RF24_CMD_PATCH_IMAGE                0x04
#define RH_RF24_CMD_FUNC_INFO                  0x10
#define RH_RF24_CMD_SET_PROPERTY               0x11
#define RH_RF24_CMD_GET_PROPERTY               0x12
#define RH_RF24_CMD_GPIO_PIN_CFG               0x13
#define RH_RF24_CMD_GET_ADC_READING            0x14
#define RH_RF24_CMD_FIFO_INFO                  0x15
#define RH_RF24_CMD_PACKET_INFO                0x16
#define RH_RF24_CMD_IRCAL                      0x17
#define RH_RF24_CMD_PROTOCOL_CFG               0x18
#define RH_RF24_CMD_GET_INT_STATUS             0x20
#define RH_RF24_CMD_GET_PH_STATUS              0x21
#define RH_RF24_CMD_GET_MODEM_STATUS           0x22
#define RH_RF24_CMD_GET_CHIP_STATUS            0x23
#define RH_RF24_CMD_START_TX                   0x31
#define RH_RF24_CMD_START_RX                   0x32
#define RH_RF24_CMD_REQUEST_DEVICE_STATE       0x33
#define RH_RF24_CMD_CHANGE_STATE               0x34
#define RH_RF24_CMD_RX_HOP                     0x36
#define RH_RF24_CMD_READ_BUF                   0x44
#define RH_RF24_CMD_FAST_RESPONSE_A            0x50
#define RH_RF24_CMD_FAST_RESPONSE_B            0x51
#define RH_RF24_CMD_FAST_RESPONSE_C            0x53
#define RH_RF24_CMD_FAST_RESPONSE_D            0x57
#define RH_RF24_CMD_TX_FIFO_WRITE              0x66
#define RH_RF24_CMD_RX_FIFO_READ               0x77

// The Clear To Send signal from the radio
#define RH_RF24_REPLY_CTS                      0xff

//#define RH_RF24_CMD_START_TX                   0x31
#define RH_RF24_CONDITION_TX_COMPLETE_STATE      0xf0
#define RH_RF24_CONDITION_RETRANSMIT_NO          0x00
#define RH_RF24_CONDITION_RETRANSMIT_YES         0x04
#define RH_RF24_CONDITION_START_IMMEDIATE        0x00
#define RH_RF24_CONDITION_START_AFTER_WUT        0x01

//#define RH_RF24_CMD_START_RX                   0x32
#define RH_RF24_CONDITION_RX_START_IMMEDIATE     0x00

//#define RH_RF24_CMD_REQUEST_DEVICE_STATE       0x33
#define RH_RF24_DEVICE_STATE_NO_CHANGE           0x00
#define RH_RF24_DEVICE_STATE_SLEEP               0x01
#define RH_RF24_DEVICE_STATE_SPI_ACTIVE          0x02
#define RH_RF24_DEVICE_STATE_READY               0x03
#define RH_RF24_DEVICE_STATE_ALSO_READY          0x04
#define RH_RF24_DEVICE_STATE_TUNE_TX             0x05
#define RH_RF24_DEVICE_STATE_TUNE_RX             0x06
#define RH_RF24_DEVICE_STATE_TX                  0x07
#define RH_RF24_DEVICE_STATE_RX                  0x08

// Properties for API Description AN625 Section 2.2
#define RH_RF24_PROPERTY_GLOBAL_XO_TUNE                   0x0000
#define RH_RF24_PROPERTY_GLOBAL_CLK_CFG                   0x0001
#define RH_RF24_PROPERTY_GLOBAL_LOW_BATT_THRESH           0x0002
#define RH_RF24_PROPERTY_GLOBAL_CONFIG                    0x0003
#define RH_RF24_PROPERTY_GLOBAL_WUT_CONFIG                0x0004
#define RH_RF24_PROPERTY_GLOBAL_WUT_M_15_8                0x0005
#define RH_RF24_PROPERTY_GLOBAL_WUT_M_7_0                 0x0006
#define RH_RF24_PROPERTY_GLOBAL_WUT_R                     0x0007
#define RH_RF24_PROPERTY_GLOBAL_WUT_LDC                   0x0008
#define RH_RF24_PROPERTY_INT_CTL_ENABLE                   0x0100
#define RH_RF24_PROPERTY_INT_CTL_PH_ENABLE                0x0101
#define RH_RF24_PROPERTY_INT_CTL_MODEM_ENABLE             0x0102
#define RH_RF24_PROPERTY_INT_CTL_CHIP_ENABLE              0x0103
#define RH_RF24_PROPERTY_FRR_CTL_A_MODE                   0x0200
#define RH_RF24_PROPERTY_FRR_CTL_B_MODE                   0x0201
#define RH_RF24_PROPERTY_FRR_CTL_C_MODE                   0x0202
#define RH_RF24_PROPERTY_FRR_CTL_D_MODE                   0x0203
#define RH_RF24_PROPERTY_PREAMBLE_TX_LENGTH               0x1000
#define RH_RF24_PROPERTY_PREAMBLE_CONFIG_STD_1            0x1001
#define RH_RF24_PROPERTY_PREAMBLE_CONFIG_NSTD             0x1002
#define RH_RF24_PROPERTY_PREAMBLE_CONFIG_STD_2            0x1003
#define RH_RF24_PROPERTY_PREAMBLE_CONFIG                  0x1004
#define RH_RF24_PROPERTY_PREAMBLE_PATTERN_31_24           0x1005
#define RH_RF24_PROPERTY_PREAMBLE_PATTERN_23_16           0x1006
#define RH_RF24_PROPERTY_PREAMBLE_PATTERN_15_8            0x1007
#define RH_RF24_PROPERTY_PREAMBLE_PATTERN_7_0             0x1008
#define RH_RF24_PROPERTY_SYNC_CONFIG                      0x1100
#define RH_RF24_PROPERTY_SYNC_BITS_31_24                  0x1101
#define RH_RF24_PROPERTY_SYNC_BITS_23_16                  0x1102
#define RH_RF24_PROPERTY_SYNC_BITS_15_8                   0x1103
#define RH_RF24_PROPERTY_SYNC_BITS_7_0                    0x1104
#define RH_RF24_PROPERTY_PKT_CRC_CONFIG                   0x1200
#define RH_RF24_PROPERTY_PKT_CONFIG1                      0x1206
#define RH_RF24_PROPERTY_PKT_LEN                          0x1208
#define RH_RF24_PROPERTY_PKT_LEN_FIELD_SOURCE             0x1209
#define RH_RF24_PROPERTY_PKT_LEN_ADJUST                   0x120a
#define RH_RF24_PROPERTY_PKT_TX_THRESHOLD                 0x120b
#define RH_RF24_PROPERTY_PKT_RX_THRESHOLD                 0x120c
#define RH_RF24_PROPERTY_PKT_FIELD_1_LENGTH_12_8          0x120d
#define RH_RF24_PROPERTY_PKT_FIELD_1_LENGTH_7_0           0x120e
#define RH_RF24_PROPERTY_PKT_FIELD_1_CONFIG               0x120f
#define RH_RF24_PROPERTY_PKT_FIELD_1_CRC_CONFIG           0x1210
#define RH_RF24_PROPERTY_PKT_FIELD_2_LENGTH_12_8          0x1211
#define RH_RF24_PROPERTY_PKT_FIELD_2_LENGTH_7_0           0x1212
#define RH_RF24_PROPERTY_PKT_FIELD_2_CONFIG               0x1213
#define RH_RF24_PROPERTY_PKT_FIELD_2_CRC_CONFIG           0x1214
#define RH_RF24_PROPERTY_PKT_FIELD_3_LENGTH_12_8          0x1215
#define RH_RF24_PROPERTY_PKT_FIELD_3_LENGTH_7_0           0x1216
#define RH_RF24_PROPERTY_PKT_FIELD_3_CONFIG               0x1217
#define RH_RF24_PROPERTY_PKT_FIELD_3_CRC_CONFIG           0x1218
#define RH_RF24_PROPERTY_PKT_FIELD_4_LENGTH_12_8          0x1219
#define RH_RF24_PROPERTY_PKT_FIELD_4_LENGTH_7_0           0x121a
#define RH_RF24_PROPERTY_PKT_FIELD_4_CONFIG               0x121b
#define RH_RF24_PROPERTY_PKT_FIELD_4_CRC_CONFIG           0x121c
#define RH_RF24_PROPERTY_PKT_FIELD_5_LENGTH_12_8          0x121d
#define RH_RF24_PROPERTY_PKT_FIELD_5_LENGTH_7_0           0x121e
#define RH_RF24_PROPERTY_PKT_FIELD_5_CONFIG               0x121f
#define RH_RF24_PROPERTY_PKT_FIELD_5_CRC_CONFIG           0x1220
#define RH_RF24_PROPERTY_PKT_RX_FIELD_1_LENGTH_12_8       0x1221
#define RH_RF24_PROPERTY_PKT_RX_FIELD_1_LENGTH_7_0        0x1222
#define RH_RF24_PROPERTY_PKT_RX_FIELD_1_CONFIG            0x1223
#define RH_RF24_PROPERTY_PKT_RX_FIELD_1_CRC_CONFIG        0x1224
#define RH_RF24_PROPERTY_PKT_RX_FIELD_2_LENGTH_12_8       0x1225
#define RH_RF24_PROPERTY_PKT_RX_FIELD_2_LENGTH_7_0        0x1226
#define RH_RF24_PROPERTY_PKT_RX_FIELD_2_CONFIG            0x1227
#define RH_RF24_PROPERTY_PKT_RX_FIELD_2_CRC_CONFIG        0x1228
#define RH_RF24_PROPERTY_PKT_RX_FIELD_3_LENGTH_12_8       0x1229
#define RH_RF24_PROPERTY_PKT_RX_FIELD_3_LENGTH_7_0        0x122a
#define RH_RF24_PROPERTY_PKT_RX_FIELD_3_CONFIG            0x122b
#define RH_RF24_PROPERTY_PKT_RX_FIELD_3_CRC_CONFIG        0x122c
#define RH_RF24_PROPERTY_PKT_RX_FIELD_4_LENGTH_12_8       0x122d
#define RH_RF24_PROPERTY_PKT_RX_FIELD_4_LENGTH_7_0        0x122e
#define RH_RF24_PROPERTY_PKT_RX_FIELD_4_CONFIG            0x122f
#define RH_RF24_PROPERTY_PKT_RX_FIELD_4_CRC_CONFIG        0x1230
#define RH_RF24_PROPERTY_PKT_RX_FIELD_5_LENGTH_12_8       0x1231
#define RH_RF24_PROPERTY_PKT_RX_FIELD_5_LENGTH_7_0        0x1232
#define RH_RF24_PROPERTY_PKT_RX_FIELD_5_CONFIG            0x1233
#define RH_RF24_PROPERTY_PKT_RX_FIELD_5_CRC_CONFIG        0x1234
#define RH_RF24_PROPERTY_MODEM_MOD_TYPE                   0x2000
#define RH_RF24_PROPERTY_MODEM_MAP_CONTROL                0x2001
#define RH_RF24_PROPERTY_MODEM_DSM_CTRL                   0x2002
#define RH_RF24_PROPERTY_MODEM_DATA_RATE_2                0x2003
#define RH_RF24_PROPERTY_MODEM_DATA_RATE_1                0x2004
#define RH_RF24_PROPERTY_MODEM_DATA_RATE_0                0x2005
#define RH_RF24_PROPERTY_MODEM_TX_NCO_MODE_3              0x2006
#define RH_RF24_PROPERTY_MODEM_TX_NCO_MODE_2              0x2007
#define RH_RF24_PROPERTY_MODEM_TX_NCO_MODE_1              0x2008
#define RH_RF24_PROPERTY_MODEM_TX_NCO_MODE_0              0x2009
#define RH_RF24_PROPERTY_MODEM_FREQ_DEV_2                 0x200a
#define RH_RF24_PROPERTY_MODEM_FREQ_DEV_1                 0x200b
#define RH_RF24_PROPERTY_MODEM_FREQ_DEV_0                 0x200c
#define RH_RF24_PROPERTY_MODEM_TX_RAMP_DELAY              0x2018
#define RH_RF24_PROPERTY_MODEM_MDM_CTRL                   0x2019
#define RH_RF24_PROPERTY_MODEM_IF_CONTROL                 0x201a
#define RH_RF24_PROPERTY_MODEM_IF_FREQ_2                  0x201b
#define RH_RF24_PROPERTY_MODEM_IF_FREQ_1                  0x201c
#define RH_RF24_PROPERTY_MODEM_IF_FREQ_0                  0x201d
#define RH_RF24_PROPERTY_MODEM_DECIMATION_CFG1            0x201e
#define RH_RF24_PROPERTY_MODEM_DECIMATION_CFG0            0x201f
#define RH_RF24_PROPERTY_MODEM_BCR_OSR_1                  0x2022
#define RH_RF24_PROPERTY_MODEM_BCR_OSR_0                  0x2023
#define RH_RF24_PROPERTY_MODEM_BCR_NCO_OFFSET_2           0x2024
#define RH_RF24_PROPERTY_MODEM_BCR_NCO_OFFSET_1           0x2025
#define RH_RF24_PROPERTY_MODEM_BCR_NCO_OFFSET_0           0x2026
#define RH_RF24_PROPERTY_MODEM_BCR_GAIN_1                 0x2027
#define RH_RF24_PROPERTY_MODEM_BCR_GAIN_0                 0x2028
#define RH_RF24_PROPERTY_MODEM_BCR_GEAR                   0x2029
#define RH_RF24_PROPERTY_MODEM_BCR_MISC1                  0x202a
#define RH_RF24_PROPERTY_MODEM_AFC_GEAR                   0x202c
#define RH_RF24_PROPERTY_MODEM_AFC_WAIT                   0x202d
#define RH_RF24_PROPERTY_MODEM_AFC_GAIN_1                 0x202e
#define RH_RF24_PROPERTY_MODEM_AFC_GAIN_0                 0x202f
#define RH_RF24_PROPERTY_MODEM_AFC_LIMITER_1              0x2030
#define RH_RF24_PROPERTY_MODEM_AFC_LIMITER_0              0x2031
#define RH_RF24_PROPERTY_MODEM_AFC_MISC                   0x2032
#define RH_RF24_PROPERTY_MODEM_AGC_CONTROL                0x2035
#define RH_RF24_PROPERTY_MODEM_AGC_WINDOW_SIZE            0x2038
#define RH_RF24_PROPERTY_MODEM_AGC_RFPD_DECAY             0x2039
#define RH_RF24_PROPERTY_MODEM_AGC_IFPD_DECAY             0x203a
#define RH_RF24_PROPERTY_MODEM_FSK4_GAIN1                 0x203b
#define RH_RF24_PROPERTY_MODEM_FSK4_GAIN0                 0x203c
#define RH_RF24_PROPERTY_MODEM_FSK4_TH1                   0x203d
#define RH_RF24_PROPERTY_MODEM_FSK4_TH0                   0x203e
#define RH_RF24_PROPERTY_MODEM_FSK4_MAP                   0x203f
#define RH_RF24_PROPERTY_MODEM_OOK_PDTC                   0x2040
#define RH_RF24_PROPERTY_MODEM_OOK_CNT1                   0x2042
#define RH_RF24_PROPERTY_MODEM_OOK_MISC                   0x2043
#define RH_RF24_PROPERTY_MODEM_RAW_SEARCH                 0x2044
#define RH_RF24_PROPERTY_MODEM_RAW_CONTROL                0x2045
#define RH_RF24_PROPERTY_MODEM_RAW_EYE_1                  0x2046
#define RH_RF24_PROPERTY_MODEM_RAW_EYE_0                  0x2047
#define RH_RF24_PROPERTY_MODEM_ANT_DIV_MODE               0x2048
#define RH_RF24_PROPERTY_MODEM_ANT_DIV_CONTROL            0x2049
#define RH_RF24_PROPERTY_MODEM_RSSI_THRESH                0x204a
#define RH_RF24_PROPERTY_MODEM_RSSI_JUMP_THRESH           0x204b
#define RH_RF24_PROPERTY_MODEM_RSSI_CONTROL               0x204c
#define RH_RF24_PROPERTY_MODEM_RSSI_CONTROL2              0x204d
#define RH_RF24_PROPERTY_MODEM_RSSI_COMP                  0x204e
#define RH_RF24_PROPERTY_MODEM_ANT_DIV_CONT               0x2049
#define RH_RF24_PROPERTY_MODEM_CLKGEN_BAND                0x2051
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE13_7_0  0x2100
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE12_7_0  0x2101
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE11_7_0  0x2102
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE10_7_0  0x2103
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE9_7_0   0x2104
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE8_7_0   0x2105
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE7_7_0   0x2106
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE6_7_0   0x2107
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE5_7_0   0x2108
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE4_7_0   0x2109
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE3_7_0   0x210a
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE2_7_0   0x210b
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE1_7_0   0x210c
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE0_7_0   0x210d
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COEM0      0x210e
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COEM1      0x210f
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COEM2      0x2110
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COEM3      0x2111
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE13_7_0  0x2112
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE12_7_0  0x2113
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE11_7_0  0x2114
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE10_7_0  0x2115
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE9_7_0   0x2116
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE8_7_0   0x2117
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE7_7_0   0x2118
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE6_7_0   0x2119
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE5_7_0   0x211a
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE4_7_0   0x211b
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE3_7_0   0x211c
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE2_7_0   0x211d
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE1_7_0   0x211e
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE0_7_0   0x211f
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COEM0      0x2120
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COEM1      0x2121
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COEM2      0x2122
#define RH_RF24_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COEM3      0x2123
#define RH_RF24_PROPERTY_PA_MODE                          0x2200
#define RH_RF24_PROPERTY_PA_PWR_LVL                       0x2201
#define RH_RF24_PROPERTY_PA_BIAS_CLKDUTY                  0x2202
#define RH_RF24_PROPERTY_PA_TC                            0x2203
#define RH_RF24_PROPERTY_SYNTH_PFDCP_CPFF                 0x2300
#define RH_RF24_PROPERTY_SYNTH_PFDCP_CPINT                0x2301
#define RH_RF24_PROPERTY_SYNTH_VCO_KV                     0x2302
#define RH_RF24_PROPERTY_SYNTH_LPFILT3                    0x2303
#define RH_RF24_PROPERTY_SYNTH_LPFILT2                    0x2304
#define RH_RF24_PROPERTY_SYNTH_LPFILT1                    0x2305
#define RH_RF24_PROPERTY_SYNTH_LPFILT0                    0x2306
#define RH_RF24_PROPERTY_MATCH_VALUE_1                    0x3000
#define RH_RF24_PROPERTY_MATCH_MASK_1                     0x3001
#define RH_RF24_PROPERTY_MATCH_CTRL_1                     0x3002
#define RH_RF24_PROPERTY_MATCH_VALUE_2                    0x3003
#define RH_RF24_PROPERTY_MATCH_MASK_2                     0x3004
#define RH_RF24_PROPERTY_MATCH_CTRL_2                     0x3005
#define RH_RF24_PROPERTY_MATCH_VALUE_3                    0x3006
#define RH_RF24_PROPERTY_MATCH_MASK_3                     0x3007
#define RH_RF24_PROPERTY_MATCH_CTRL_3                     0x3008
#define RH_RF24_PROPERTY_MATCH_VALUE_4                    0x3009
#define RH_RF24_PROPERTY_MATCH_MASK_4                     0x300a
#define RH_RF24_PROPERTY_MATCH_CTRL_4                     0x300b
#define RH_RF24_PROPERTY_FREQ_CONTROL_INTE                0x4000
#define RH_RF24_PROPERTY_FREQ_CONTROL_FRAC_2              0x4001
#define RH_RF24_PROPERTY_FREQ_CONTROL_FRAC_1              0x4002
#define RH_RF24_PROPERTY_FREQ_CONTROL_FRAC_0              0x4003
#define RH_RF24_PROPERTY_FREQ_CONTROL_CHANNEL_STEP_SIZE_1 0x4004
#define RH_RF24_PROPERTY_FREQ_CONTROL_CHANNEL_STEP_SIZE_0 0x4005
#define RH_RF24_PROPERTY_FREQ_CONTROL_VCOCNT_RX_ADJ       0x4007
#define RH_RF24_PROPERTY_RX_HOP_CONTROL                   0x5000
#define RH_RF24_PROPERTY_RX_HOP_TABLE_SIZE                0x5001
#define RH_RF24_PROPERTY_RX_HOP_TABLE_ENTRY_0             0x5002

//#define RH_RF24_CMD_GPIO_PIN_CFG               0x13
#define RH_RF24_GPIO_NO_CHANGE                   0
#define RH_RF24_GPIO_DISABLED                    1
#define RH_RF24_GPIO_LOW                         2
#define RH_RF24_GPIO_HIGH                        3
#define RH_RF24_GPIO_INPUT                       4
#define RH_RF24_GPIO_32_KHZ_CLOCK                5
#define RH_RF24_GPIO_BOOT_CLOCK                  6
#define RH_RF24_GPIO_DIVIDED_MCU_CLOCK           7
#define RH_RF24_GPIO_CTS                         8
#define RH_RF24_GPIO_INV_CTS                     9
#define RH_RF24_GPIO_HIGH_ON_CMD_OVERLAP         10
#define RH_RF24_GPIO_SPI_DATA_OUT                11
#define RH_RF24_GPIO_HIGH_AFTER_RESET            12
#define RH_RF24_GPIO_HIGH_AFTER_CALIBRATION      13
#define RH_RF24_GPIO_HIGH_AFTER_WUT              14
#define RH_RF24_GPIO_UNUSED_0                    15
#define RH_RF24_GPIO_TX_DATA_CLOCK               16
#define RH_RF24_GPIO_RX_DATA_CLOCK               17
#define RH_RF24_GPIO_UNUSED_1                    18
#define RH_RF24_GPIO_TX_DATA                     19
#define RH_RF24_GPIO_RX_DATA                     20
#define RH_RF24_GPIO_RX_RAW_DATA                 21
#define RH_RF24_GPIO_ANTENNA_1_SWITCH            22
#define RH_RF24_GPIO_ANTENNA_2_SWITCH            23
#define RH_RF24_GPIO_VALID_PREAMBLE              24
#define RH_RF24_GPIO_INVALID_PREAMBLE            25
#define RH_RF24_GPIO_SYNC_DETECTED               26
#define RH_RF24_GPIO_RSSI_ABOVE_CAT              27
#define RH_RF24_GPIO_TX_STATE                    32
#define RH_RF24_GPIO_RX_STATE                    33
#define RH_RF24_GPIO_RX_FIFO_ALMOST_FULL         34
#define RH_RF24_GPIO_TX_FIFO_ALMOST_EMPTY        35
#define RH_RF24_GPIO_BATT_LOW                    36
#define RH_RF24_GPIO_RSSI_ABOVE_CAT_LOW          37
#define RH_RF24_GPIO_HOP                         38
#define RH_RF24_GPIO_HOP_TABLE_WRAPPED           39

// #define RH_RF24_CMD_GET_INT_STATUS             0x20
#define RH_RF24_INT_STATUS_CHIP_INT_STATUS                0x04
#define RH_RF24_INT_STATUS_MODEM_INT_STATUS               0x02
#define RH_RF24_INT_STATUS_PH_INT_STATUS                  0x01
#define RH_RF24_INT_STATUS_FILTER_MATCH                   0x80
#define RH_RF24_INT_STATUS_FILTER_MISS                    0x40
#define RH_RF24_INT_STATUS_PACKET_SENT                    0x20
#define RH_RF24_INT_STATUS_PACKET_RX                      0x10
#define RH_RF24_INT_STATUS_CRC_ERROR                      0x08
#define RH_RF24_INT_STATUS_TX_FIFO_ALMOST_EMPTY           0x02
#define RH_RF24_INT_STATUS_RX_FIFO_ALMOST_FULL            0x01
#define RH_RF24_INT_STATUS_INVALID_SYNC                   0x20
#define RH_RF24_INT_STATUS_RSSI_JUMP                      0x10
#define RH_RF24_INT_STATUS_RSSI                           0x08
#define RH_RF24_INT_STATUS_INVALID_PREAMBLE               0x04
#define RH_RF24_INT_STATUS_PREAMBLE_DETECT                0x02
#define RH_RF24_INT_STATUS_SYNC_DETECT                    0x01
#define RH_RF24_INT_STATUS_CAL                            0x40
#define RH_RF24_INT_STATUS_FIFO_UNDERFLOW_OVERFLOW_ERROR  0x20
#define RH_RF24_INT_STATUS_STATE_CHANGE                   0x10
#define RH_RF24_INT_STATUS_CMD_ERROR                      0x08
#define RH_RF24_INT_STATUS_CHIP_READY                     0x04
#define RH_RF24_INT_STATUS_LOW_BATT                       0x02
#define RH_RF24_INT_STATUS_WUT                            0x01

//#define RH_RF24_PROPERTY_GLOBAL_CLK_CFG                   0x0001
#define RH_RF24_CLK_CFG_DIVIDED_CLK_EN                    0x40
#define RH_RF24_CLK_CFG_DIVIDED_CLK_SEL_30                0x30
#define RH_RF24_CLK_CFG_DIVIDED_CLK_SEL_15                0x28
#define RH_RF24_CLK_CFG_DIVIDED_CLK_SEL_10                0x20
#define RH_RF24_CLK_CFG_DIVIDED_CLK_SEL_7_5               0x18
#define RH_RF24_CLK_CFG_DIVIDED_CLK_SEL_3                 0x10
#define RH_RF24_CLK_CFG_DIVIDED_CLK_SEL_2                 0x08
#define RH_RF24_CLK_CFG_DIVIDED_CLK_SEL_1                 0x00
#define RH_RF24_CLK_CFG_CLK_32K_SEL_EXTERNAL              0x02
#define RH_RF24_CLK_CFG_CLK_32K_SEL_RC                    0x01
#define RH_RF24_CLK_CFG_CLK_32K_SEL_DISABLED              0x00

//#define RH_RF24_PROPERTY_FRR_CTL_A_MODE                   0x0200
//#define RH_RF24_PROPERTY_FRR_CTL_B_MODE                   0x0201
//#define RH_RF24_PROPERTY_FRR_CTL_C_MODE                   0x0202
//#define RH_RF24_PROPERTY_FRR_CTL_D_MODE                   0x0203
#define RH_RF24_FRR_MODE_DISABLED                         0
#define RH_RF24_FRR_MODE_GLOBAL_STATUS                    1
#define RH_RF24_FRR_MODE_GLOBAL_INTERRUPT_PENDING         2
#define RH_RF24_FRR_MODE_PACKET_HANDLER_STATUS            3
#define RH_RF24_FRR_MODE_PACKET_HANDLER_INTERRUPT_PENDING 4
#define RH_RF24_FRR_MODE_MODEM_STATUS                     5
#define RH_RF24_FRR_MODE_MODEM_INTERRUPT_PENDING          6
#define RH_RF24_FRR_MODE_CHIP_STATUS                      7
#define RH_RF24_FRR_MODE_CHIP_INTERRUPT_PENDING           8
#define RH_RF24_FRR_MODE_CURRENT_STATE                    9
#define RH_RF24_FRR_MODE_LATCHED_RSSI                     10

//#define RH_RF24_PROPERTY_INT_CTL_ENABLE                   0x0100
#define RH_RF24_CHIP_INT_STATUS_EN                        0x04
#define RH_RF24_MODEM_INT_STATUS_EN                       0x02
#define RH_RF24_PH_INT_STATUS_EN                          0x01

//#define RH_RF24_PROPERTY_PREAMBLE_CONFIG                  0x1004
#define RH_RF24_PREAMBLE_FIRST_1                          0x20
#define RH_RF24_PREAMBLE_FIRST_0                          0x00
#define RH_RF24_PREAMBLE_LENGTH_NIBBLES                   0x00
#define RH_RF24_PREAMBLE_LENGTH_BYTES                     0x10
#define RH_RF24_PREAMBLE_MAN_CONST                        0x08
#define RH_RF24_PREAMBLE_MAN_ENABLE                       0x02
#define RH_RF24_PREAMBLE_NON_STANDARD                     0x00
#define RH_RF24_PREAMBLE_STANDARD_1010                    0x01
#define RH_RF24_PREAMBLE_STANDARD_0101                    0x02

//#define RH_RF24_PROPERTY_SYNC_CONFIG                      0x1100
#define RH_RF24_SYNC_CONFIG_SKIP_TX                       0x80
#define RH_RF24_SYNC_CONFIG_RX_ERRORS_MASK                0x70
#define RH_RF24_SYNC_CONFIG_4FSK                          0x08
#define RH_RF24_SYNC_CONFIG_MANCH                         0x04
#define RH_RF24_SYNC_CONFIG_LENGTH_MASK                   0x03

//#define RH_RF24_PROPERTY_PKT_CRC_CONFIG                   0x1200
#define RH_RF24_CRC_SEED_ALL_0S                           0x00
#define RH_RF24_CRC_SEED_ALL_1S                           0x80
#define RH_RF24_CRC_MASK                                  0x0f
#define RH_RF24_CRC_NONE                                  0x00
#define RH_RF24_CRC_ITU_T                                 0x01
#define RH_RF24_CRC_IEC_16                                0x02
#define RH_RF24_CRC_BIACHEVA                              0x03
#define RH_RF24_CRC_16_IBM                                0x04
#define RH_RF24_CRC_CCITT                                 0x05
#define RH_RF24_CRC_KOOPMAN                               0x06
#define RH_RF24_CRC_IEEE_802_3                            0x07
#define RH_RF24_CRC_CASTAGNOLI                            0x08

//#define RH_RF24_PROPERTY_PKT_CONFIG1                      0x1206
#define RH_RF24_PH_FIELD_SPLIT                            0x80
#define RH_RF24_PH_RX_DISABLE                             0x40
#define RH_RF24_4FSK_EN                                   0x20
#define RH_RF24_RX_MULTI_PKT                              0x10
#define RH_RF24_MANCH_POL                                 0x08
#define RH_RF24_CRC_INVERT                                0x04
#define RH_RF24_CRC_ENDIAN                                0x02
#define RH_RF24_BIT_ORDER                                 0x01

//#define RH_RF24_PROPERTY_PKT_FIELD_1_CONFIG               0x120f
//#define RH_RF24_PROPERTY_PKT_FIELD_2_CONFIG               0x1213
//#define RH_RF24_PROPERTY_PKT_FIELD_3_CONFIG               0x1217
//#define RH_RF24_PROPERTY_PKT_FIELD_4_CONFIG               0x121b
//#define RH_RF24_PROPERTY_PKT_FIELD_5_CONFIG               0x121f
#define RH_RF24_FIELD_CONFIG_4FSK                         0x10
#define RH_RF24_FIELD_CONFIG_WHITEN                       0x02
#define RH_RF24_FIELD_CONFIG_MANCH                        0x01

//#define RH_RF24_PROPERTY_PKT_RX_FIELD_1_CRC_CONFIG        0x1224
//#define RH_RF24_PROPERTY_PKT_RX_FIELD_2_CRC_CONFIG        0x1228
//#define RH_RF24_PROPERTY_PKT_RX_FIELD_3_CRC_CONFIG        0x122c
//#define RH_RF24_PROPERTY_PKT_RX_FIELD_4_CRC_CONFIG        0x1230
//#define RH_RF24_PROPERTY_PKT_RX_FIELD_5_CRC_CONFIG        0x1234
#define RH_RF24_FIELD_CONFIG_CRC_START                     0x80
#define RH_RF24_FIELD_CONFIG_SEND_CRC                      0x20
#define RH_RF24_FIELD_CONFIG_CHECK_CRC                     0x08
#define RH_RF24_FIELD_CONFIG_CRC_ENABLE                    0x02




//#define RH_RF24_PROPERTY_MODEM_MOD_TYPE                   0x2000
#define RH_RF24_TX_DIRECT_MODE_TYPE_SYNCHRONOUS           0x00
#define RH_RF24_TX_DIRECT_MODE_TYPE_ASYNCHRONOUS          0x80
#define RH_RF24_TX_DIRECT_MODE_GPIO0                      0x00
#define RH_RF24_TX_DIRECT_MODE_GPIO1                      0x20
#define RH_RF24_TX_DIRECT_MODE_GPIO2                      0x40
#define RH_RF24_TX_DIRECT_MODE_GPIO3                      0x60
#define RH_RF24_MOD_SOURCE_PACKET_HANDLER                 0x00
#define RH_RF24_MOD_SOURCE_DIRECT_MODE                    0x08
#define RH_RF24_MOD_SOURCE_RANDOM_GENERATOR               0x10
#define RH_RF24_MOD_TYPE_CW                               0x00
#define RH_RF24_MOD_TYPE_OOK                              0x01
#define RH_RF24_MOD_TYPE_2FSK                             0x02
#define RH_RF24_MOD_TYPE_2GFSK                            0x03
#define RH_RF24_MOD_TYPE_4FSK                             0x04
#define RH_RF24_MOD_TYPE_4GFSK                            0x05

//    RH_RF24_PROPERTY_PA_MODE                          0x2200
#define RH_RF24_PA_MODE_1_GROUP                           0x04
#define RH_RF24_PA_MODE_2_GROUPS                          0x08
#define RH_RF24_PA_MODE_CLASS_E                           0x00
#define RH_RF24_PA_MODE_SWITCH_CURRENT                    0x01














class RHHardwareSPI
{
public:

typedef enum
{
DataMode0 = 0, ///< SPI Mode 0: CPOL = 0, CPHA = 0
DataMode1,     ///< SPI Mode 1: CPOL = 0, CPHA = 1
DataMode2,     ///< SPI Mode 2: CPOL = 1, CPHA = 0
DataMode3,     ///< SPI Mode 3: CPOL = 1, CPHA = 1
} DataMode;

typedef enum
{
Frequency1MHz = 0,  ///< SPI bus frequency close to 1MHz
Frequency2MHz,      ///< SPI bus frequency close to 2MHz
Frequency4MHz,      ///< SPI bus frequency close to 4MHz
Frequency8MHz,      ///< SPI bus frequency close to 8MHz
Frequency16MHz      ///< SPI bus frequency close to 16MHz
} Frequency;

typedef enum
{
BitOrderMSBFirst = 0,  ///< SPI MSB first
BitOrderLSBFirst,      ///< SPI LSB first
} BitOrder;



RHHardwareSPI(Frequency frequency = Frequency1MHz, BitOrder bitOrder = BitOrderMSBFirst, DataMode dataMode = DataMode0)
    :
        _frequency(frequency),
        _bitOrder(bitOrder),
        _dataMode(dataMode)
    {
    }

    void setBitOrder(BitOrder bitOrder)
    {
        _bitOrder = bitOrder;
    }

    void setDataMode(DataMode dataMode)
    {
        _dataMode = dataMode; 
    }

    void setFrequency(Frequency frequency)
    {
        _frequency = frequency;
    }

    uint8_t transfer(uint8_t data) 
    {
        return SPI.transfer(data);
    }


    void attachInterrupt() 
    {
        SPI.attachInterrupt();
    }

    void detachInterrupt() 
    {
        SPI.detachInterrupt();
    }
        
    void begin() 
    {
        // Perhaps this is a uniform interface for SPI?
        // Currently Teensy and ESP32 only
    uint32_t frequency;
    if (_frequency == Frequency16MHz)
        frequency = 16000000;
    else if (_frequency == Frequency8MHz)
        frequency = 8000000;
    else if (_frequency == Frequency4MHz)
        frequency = 4000000;
    else if (_frequency == Frequency2MHz)
        frequency = 2000000;
    else
        frequency = 1000000;

        uint8_t bitOrder;

    if (_bitOrder == BitOrderLSBFirst)
        bitOrder = LSBFIRST;
    else
        bitOrder = MSBFIRST;
    
        uint8_t dataMode;
        if (_dataMode == DataMode0)
        dataMode = SPI_MODE0;
        else if (_dataMode == DataMode1)
        dataMode = SPI_MODE1;
        else if (_dataMode == DataMode2)
        dataMode = SPI_MODE2;
        else if (_dataMode == DataMode3)
        dataMode = SPI_MODE3;
        else
        dataMode = SPI_MODE0;

        // Save the settings for use in transactions
    _settings = SPISettings(frequency, bitOrder, dataMode);
    SPI.begin();
    }

    void end() 
    {
        return SPI.end();
    }

    void beginTransaction()
    {
        SPI.beginTransaction(_settings);
    }

    void endTransaction()
    {
        SPI.endTransaction();
    }

    void usingInterrupt(uint8_t interrupt)
    {
        SPI.usingInterrupt(interrupt);
    }

protected:
    
    /// The configure SPI Bus frequency, one of Frequency
    Frequency    _frequency; // Bus frequency, one of Frequency

    /// Bit order, one of BitOrder
    BitOrder     _bitOrder;  

    /// SPI bus mode, one of DataMode
    DataMode     _dataMode;  

    SPISettings  _settings;
};






// This is the bit in the SPI address that marks it as a write
#define RH_SPI_WRITE_MASK 0x80



class RHSPIDriver
{
public:
    RHSPIDriver(uint8_t slaveSelectPin)
        : 
        _slaveSelectPin(slaveSelectPin)
    {
    }

    bool init()
    {
        // start the SPI library with the default speeds etc:
        // On Arduino Due this defaults to SPI1 on the central group of 6 SPI pins
        _spi.begin();

        // Initialise the slave select pin
        // On Maple, this must be _after_ spi.begin

        // Sometimes we dont want to work the _slaveSelectPin here
        if (_slaveSelectPin != 0xff)
        pinMode(_slaveSelectPin, OUTPUT);

        deselectSlave();

        // This delay is needed for ATMega and maybe some others, but
        // 100ms is too long for STM32L0, and somehow can cause the USB interface to fail
        // in some versions of the core.
        delay(100);
        
        return true;
    }

    uint8_t spiRead(uint8_t reg)
    {
        uint8_t val;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        selectSlave();
        _spi.transfer(reg & ~RH_SPI_WRITE_MASK); // Send the address with the write mask off
        val = _spi.transfer(0); // The written value is ignored, reg value is read
        deselectSlave();
        }
        return val;
    }

    uint8_t spiWrite(uint8_t reg, uint8_t val)
    {
        uint8_t status = 0;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        _spi.beginTransaction();
        selectSlave();
        status = _spi.transfer(reg | RH_SPI_WRITE_MASK); // Send the address with the write mask on
        _spi.transfer(val); // New value follows
        deselectSlave();
        _spi.endTransaction();
        }
        return status;
    }

    uint8_t spiBurstRead(uint8_t reg, uint8_t* dest, uint8_t len)
    {
        uint8_t status = 0;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        _spi.beginTransaction();
        selectSlave();
        status = _spi.transfer(reg & ~RH_SPI_WRITE_MASK); // Send the start address with the write mask off
        while (len--)
        *dest++ = _spi.transfer(0);
        deselectSlave();
        _spi.endTransaction();
        }
        return status;
    }

    uint8_t spiBurstWrite(uint8_t reg, const uint8_t* src, uint8_t len)
    {
        uint8_t status = 0;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        _spi.beginTransaction();
        selectSlave();
        status = _spi.transfer(reg | RH_SPI_WRITE_MASK); // Send the start address with the write mask on
        while (len--)
        _spi.transfer(*src++);
        deselectSlave();
        _spi.endTransaction();
        }
        return status;
    }

    void setSlaveSelectPin(uint8_t slaveSelectPin)
    {
        _slaveSelectPin = slaveSelectPin;
    }

    void spiUsingInterrupt(uint8_t interruptNumber)
    {
        _spi.usingInterrupt(interruptNumber);
    }

    void selectSlave()
    {
        digitalWrite(_slaveSelectPin, LOW);
    }
        
    void deselectSlave()
    {
        digitalWrite(_slaveSelectPin, HIGH);
    }

    /// Reference to the RHGenericSPI instance to use to transfer data with the SPI device
    RHHardwareSPI _spi;

    /// The pin number of the Slave Select pin that is used to select the desired device.
    uint8_t             _slaveSelectPin;
};












class RFSI4463PROPacket : public RHSPIDriver
{
public:

    static const uint8_t MAX_PACKET_SIZE = MAX_PAYLOAD_LEN;


private:

    static const uint32_t SELF_HEALING_INTERVAL_MS = 45000;


public:

    /// \brief Defines the available choices for CRC
    /// Types of permitted CRC polynomials, to be passed to setCRCPolynomial()
    /// They deliberately have the same numeric values as the CRC_POLYNOMIAL field of PKT_CRC_CONFIG
    typedef enum
    {
	CRC_NONE = 0,
	CRC_ITU_T,
	CRC_IEC_16,
	CRC_Biacheva,
	CRC_16_IBM,
	CRC_CCITT,
	CRC_Koopman,
	CRC_IEEE_802_3,
	CRC_Castagnoli,
    } CRCPolynomial;

    /// \brief Defines the commands we can interrogate in printRegisters
    typedef struct
    {
	uint8_t      cmd;       ///< The command number
	uint8_t      replyLen;  ///< Number of bytes in the reply stream (after the CTS)
    }   CommandInfo;

    typedef enum
    {
	RHModeInitialising = 0, ///< Transport is initialising. Initial default value until init() is called..
	RHModeSleep,            ///< Transport hardware is in low power sleep mode (if supported)
	RHModeIdle,             ///< Transport is idle.
	RHModeTx,               ///< Transport is in the process of transmitting a message.
	RHModeRx,               ///< Transport is in the process of receiving a message.
	RHModeCad               ///< Transport is in the process of detecting channel activity (if supported)
    } RHMode;


    RFSI4463PROPacket(uint8_t slaveSelectPin, uint8_t interruptPin, uint8_t sdnPin)
        :
        RHSPIDriver(PAL.GetArduinoPinFromPhysicalPin(slaveSelectPin))
        , _mode(RHModeInitialising)
        , ied_(interruptPin, LEVEL_FALLING)
    {
        _sdnPin = PAL.GetArduinoPinFromPhysicalPin(sdnPin);
        _idleMode = RH_RF24_DEVICE_STATE_READY;
    }

    void setIdleMode(uint8_t idleMode)
    {
        _idleMode = idleMode;
    }

    bool init()
    {
        _mode = RHModeInitialising;

        // Init SPI
        RHSPIDriver::init();

        // Initialise the radio
        power_on_reset();
        cmd_clear_all_interrupts();

        // Set up callbacks that need to run even if everything below fails
        // Set up interrupt handler
        ied_.SetCallback([this](uint8_t){
            if (IsOperable())
            {
                measurements_.timeUsLastISR = ied_.GetISREventTimeUs();
                handleInterrupt();
            }
            else
            {
                ReInit();
            }
        });
        ied_.RegisterForInterruptEvent();

        // Do some periodic self-healing
        ScheduleSelfHealing();

        // Get the device type and check it
        // This also tests whether we are really connected to a device
        uint8_t buf[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        if (!command(RH_RF24_CMD_PART_INFO, 0, 0, buf, sizeof(buf)))
        return false; // SPI error? Not connected?
        _deviceType = (buf[1] << 8) | buf[2];
        // Check PART to be either 0x4460, 0x4461, 0x4463, 0x4464
        if (_deviceType != 0x4460 &&
        _deviceType != 0x4461 &&
        _deviceType != 0x4463 &&
        _deviceType != 0x4464)
        return false; // Unknown radio type, or not connected

        // Here we use a configuration generated by the Silicon Labs Wireless Development Suite
        // #included above
        // We override a few things later that we ned to be sure of.
        configure(RF24_CONFIGURATION_DATA);

        // Ensure we get the interrupts we need, irrespective of whats in the radio_config
        uint8_t int_ctl[] = {RH_RF24_MODEM_INT_STATUS_EN | RH_RF24_PH_INT_STATUS_EN, 0xff, 0xff, 0x00 };
        set_properties(RH_RF24_PROPERTY_INT_CTL_ENABLE, int_ctl, sizeof(int_ctl));

        // RSSI Latching should be configured in MODEM_RSSI_CONTROL in radio_config

        // PKT_TX_THRESHOLD and PKT_RX_THRESHOLD should be set to about 0x30 in radio_config

        // Configure important RH_RF24 registers
        // Here we set up the standard packet format for use by the RH_RF24 library:
        // We will use FIFO Mode, with automatic packet generation
        // We have 2 fields:
        // Field 1 contains only the (variable) length of field 2, with CRC
        // Field 2 contains the variable length payload and the CRC
        // Hmmm, having no CRC on field 1 and CRC on field 2 causes CRC errors when resetting after an odd
        // number of packets! Anyway its prob a good thing at the cost of some airtime.
        // Hmmm, enabling WHITEN stops it working!
        uint8_t pkt_config1[] = { 0x00 };
        set_properties(RH_RF24_PROPERTY_PKT_CONFIG1, pkt_config1, sizeof(pkt_config1));

        uint8_t pkt_len[] = { 0x02, 0x01, 0x00 };
        set_properties(RH_RF24_PROPERTY_PKT_LEN, pkt_len, sizeof(pkt_len));

        uint8_t pkt_field1[] = { 0x00, 0x01, 0x00, RH_RF24_FIELD_CONFIG_CRC_START | RH_RF24_FIELD_CONFIG_SEND_CRC | RH_RF24_FIELD_CONFIG_CHECK_CRC | RH_RF24_FIELD_CONFIG_CRC_ENABLE };
        set_properties(RH_RF24_PROPERTY_PKT_FIELD_1_LENGTH_12_8, pkt_field1, sizeof(pkt_field1));

        uint8_t pkt_field2[] = { 0x00, sizeof(_buf), 0x00, RH_RF24_FIELD_CONFIG_CRC_START | RH_RF24_FIELD_CONFIG_SEND_CRC | RH_RF24_FIELD_CONFIG_CHECK_CRC | RH_RF24_FIELD_CONFIG_CRC_ENABLE };
        set_properties(RH_RF24_PROPERTY_PKT_FIELD_2_LENGTH_12_8, pkt_field2, sizeof(pkt_field2));

        // Override the above setting
        SetField2MaxSize();

        // Clear all other fields so they are never used, irrespective of the radio_config
        uint8_t pkt_fieldn[] = { 0x00, 0x00, 0x00, 0x00 };
        set_properties(RH_RF24_PROPERTY_PKT_FIELD_3_LENGTH_12_8, pkt_fieldn, sizeof(pkt_fieldn));
        set_properties(RH_RF24_PROPERTY_PKT_FIELD_4_LENGTH_12_8, pkt_fieldn, sizeof(pkt_fieldn));
        set_properties(RH_RF24_PROPERTY_PKT_FIELD_5_LENGTH_12_8, pkt_fieldn, sizeof(pkt_fieldn));

        // The following can be changed later by the user if necessary.
        // Set up default configuration
        setCRCPolynomial(CRC_16_IBM);
        uint8_t syncwords[] = { 0x2d, 0xd4 };
        setSyncWords(syncwords, sizeof(syncwords)); // Same as RF22's
        // 3 would be sufficient, but this is the same as RF22's
        // actualy, 4 seems to work much better for some modulations
        setPreambleLength(4);

        setTxPower(txPower_);

        return IsOperable();
    }

    // seen to be possible that the module depowers, re-powers, set back to
    // its original state, all without triggering any interrupts or
    // other events which get our attention to ReInit.
    //
    // That would leave a listener in a persistant state of thinking they
    // were listening but actually weren't.
    //
    // The solution is to periodically simply ReInit, knowing that it's not
    // possible to know if it was necessary or not.
    //
    // Need to balance the interruption caused by this vs the negative
    // effect of waiting to do it.
    //
    // If packets are being sent and received properly, there is no need to
    // do the reset, so monitor for signs of life and schedule time to check
    // in occasionally to reset if needed.
    // 
    // Basically a watchdog
    void ScheduleSelfHealing()
    {
        signsOfLife_ = 0;
        ted_.SetCallback([this](){
            if (!signsOfLife_)
            {
                ReInit();
            }
            else
            {
                ted_.RegisterForTimedEvent(SELF_HEALING_INTERVAL_MS);
            }
            
            signsOfLife_ = 0;
        });
        
        ted_.RegisterForTimedEvent(SELF_HEALING_INTERVAL_MS);
    }

    uint8_t ReInit()
    {
        uint8_t retVal = 0;

        // device can go inoperable.
        // that's bad.
        // OnTxComplete callbacks may never fire.
        // Sends/Listens not knowably failed.
        // Interrupts fire and consult bad statuses.
        //
        // Device seems to be able to speak SPI early on when not powered
        // properly, but when RX mode enabled, it stops working.
        // So init() doesn't catch that it's failing, because from its
        // perspective it's working.

        // So device can:
        // - not work from the start but not know it until heavy operation
        // - work initially, but stop working randomly at any point
        //   - and later come back from this


        // appraoch
        // - Allow Re-Init
        //   - cache state if listening
        //   - init
        //   - re-apply state if listening
        //   - fire ontxcomplete if that was pending
        // - return errors on API functions
        // - attempt auto-re-init from functions which notice
        //   - either as a result of being an API call
        //   - or from getting IRQs
        
        // hopefully that gets you back to where you are and gives callers
        // knowledge of state


        RFSI4463PROPacket::RHMode modeCached = _mode;

        if (init())
        {
            if (modeCached == RHModeRx)
            {
                SetModeRxInternal();
            }
            else if (modeCached == RHModeTx)
            {
                // avoid the TX call leading to a send
                //   leading to another send
                //     leading to another fail
                //       leading to another reinit
                //         leading to recursive stack crash
                ted_.SetCallback([this](){
                    txCb_();
                    ScheduleSelfHealing();
                });
                ted_.RegisterForTimedEvent(0);

                // no need to worry about real IRQ triggering callback, should
                // be in a different mode now with all interrupts cleared.
            }

            // Seen to not work on init, so if not entering RX/TX, this
            // may be a false answer too
            retVal = IsOperable();
        }
        else
        {
            retVal = 0;

            _mode = modeCached;
        }

        return retVal;
    }

    // C++ level interrupt handler for this instance
    void handleInterrupt()
    {
        uint8_t status[8];
        command(RH_RF24_CMD_GET_INT_STATUS, NULL, 0, status, sizeof(status));

        // Decode and handle the interrupt bits we are interested in
        if (status[0] & RH_RF24_INT_STATUS_CHIP_INT_STATUS)
        {
            // Log("--CHIP--");
            // Log("INT_STATUS   : ", LogBIN(status[0]));
            // Log("PH_PENDING   : ", LogBIN(status[2]));
            // Log("MODEM_PENDING: ", LogBIN(status[4]));
            // Log("CHIP_PENDING : ", LogBIN(status[6]));
            // LogNL();
        }
        if (status[0] & RH_RF24_INT_STATUS_MODEM_INT_STATUS)
        {
            ++stats_.countIrqModem;

        //	if (status[4] & RH_RF24_INT_STATUS_INVALID_PREAMBLE)
            if (status[4] & RH_RF24_INT_STATUS_INVALID_SYNC)
            {
                ++stats_.countErrInvalidSync;

                // After INVALID_SYNC, sometimes the radio gets into a silly state and subsequently reports it for every packet
                // Need to reset the radio and clear the RX FIFO, cause sometimes theres junk there too
                _mode = RHModeIdle;
                clearRxFifo();
                setModeRx();
            }

            // if (!(status[4] & RH_RF24_INT_STATUS_INVALID_SYNC) &&
            //     !(status[4] & RH_RF24_INT_STATUS_RSSI) &&
            //     !(status[4] & RH_RF24_INT_STATUS_PREAMBLE_DETECT) &&
            //     !(status[4] & RH_RF24_INT_STATUS_SYNC_DETECT))
            // {
            //     Log("--MODEM--");
            //     Log("INT_STATUS   : ", LogBIN(status[0]));
            //     Log("PH_PENDING   : ", LogBIN(status[2]));
            //     Log("MODEM_PENDING: ", LogBIN(status[4]));
            //     Log("CHIP_PENDING : ", LogBIN(status[6]));
            //     LogNL();
            // }

        }
        
        if (status[0] & RH_RF24_INT_STATUS_PH_INT_STATUS)
        {
            ++stats_.countIrqPh;

            // if
            // ( 
            //     !(status[2] & RH_RF24_INT_STATUS_CRC_ERROR) &&
            //     !(status[2] & RH_RF24_INT_STATUS_PACKET_SENT) &&
            //     !(status[2] & RH_RF24_INT_STATUS_TX_FIFO_ALMOST_EMPTY) &&
            //     !(status[2] & RH_RF24_INT_STATUS_RX_FIFO_ALMOST_FULL) &&
            //     !(status[2] & RH_RF24_INT_STATUS_PACKET_RX)
            // )
            // {
            //     Log("--PH--");
            //     Log("INT_STATUS   : ", LogBIN(status[0]));
            //     Log("PH_PENDING   : ", LogBIN(status[2]));
            //     Log("MODEM_PENDING: ", LogBIN(status[4]));
            //     Log("CHIP_PENDING : ", LogBIN(status[6]));
            //     LogNL();
            // }

            if (status[2] & RH_RF24_INT_STATUS_CRC_ERROR)
            {
                ++stats_.countErrCrc;

                // CRC Error

                // Radio automatically went to _idleMode
                _mode = RHModeIdle;

                clearRxFifo();

                // get back to listening.  you must have been already or you
                // wouldn't be receiving and decoding CRCs.
                setModeRx();

                // ok I've seen this now too
                // what to do in this case?
                // doesn't seem to be seeing new packets
                    // not actually in rx mode anymore at this point

                // also don't pass up a junked up CRC packet...
                // so get some elseifs
                    // Actually no, registers indicate CRC pass, I think it's
                    // something wrong on the sender side

                // also on the receiver side, the CRCs seem to stay junked?
                // like seemingly good sends don't decode anymore?
                    // what makes you think it's good on the send side?
                        // nothing changed?
                        // if I do a reset on receiver it works again(?)
                            // no... trashed on sender side seemingly
                                // when?  what happened?  how to tell?
                                // was logging, probably slowed down the final
                                // byte of 64 char send, fucked up CRC calculator,
                                // and I bet the CRC calculator doesn't reset for
                                // each packet.
                                    // Nope!  ok, so then I don't get it.  I think
                                    // I trashed the internal state of the chip somehow when
                                    // I let the TX queue drain and then putting in a bit more data.
                                    // so I think from then on there's garbage in the queue.
                                    // if I were able to clearTxFifo (can't) I bet this would sort it
                                        // actually that doesn't appear to be true either.
                                        // seems to only happen when I underflow on second packet (65th byte)
                                        // that messes up CRCs.  If I underflow (on purpose) on the first packet
                                        // the CRC check passes on receiver, but the data is garbage, due (I think)
                                        // to the sender chip making up data to send and CRCing it (correctly).
                
                // Solution -- single packet transmissions.
                // No single transmission can span more than a single TX queue.
                // Meaning our buffer must be smaller than the chip FIFO.
                // This is fine.

            }

            if (status[2] & RH_RF24_INT_STATUS_PACKET_SENT)
            {
                // Transmission does not automatically clear the tx buffer.
                // Could retransmit if we wanted
                // RH_RF24 configured to transition automatically to Idle after packet sent
                _mode = RHModeIdle;

                measurements_.timeUsPacketTxComplete = measurements_.timeUsLastISR;

                ++stats_.countPacketsTx;
                stats_.countBytesTx += countBytesRxQueued_;

                // on transmit complete callback
                txCb_();

                // Reset timeout after a clear sign the module is functioning
                signsOfLife_ = 1;
            }

            if (status[2] & RH_RF24_INT_STATUS_PACKET_RX)
            {
                // A complete message has been received with good CRC

                // Radio will have transitioned automatically to the _idleMode
                _mode = RHModeIdle;

                // Look at what the module has for a packet size
                uint8_t fifoLen = ReadFifoLen();

                // Check if the packet received can fit in our buffer
                if (fifoLen > MAX_PAYLOAD_LEN)
                {
                    ++stats_.countErrRxOverflow;

                    // Overflow pending
                    setModeIdle();
                    clearRxFifo();

                    // get back to listening.  you must have been already or you
                    // wouldn't be receiving and decoding CRCs.
                    setModeRx();
                }
                else
                {
                    ++stats_.countPacketsRx;
                    stats_.countBytesRx += fifoLen;

                    // We have capacity to receive the packet

                    // Receive data into buffer
                    readRxFifo(_buf, fifoLen);

                    // Get the RSSI, configured to latch at sync detect in radio_config
                    measurements_.rssi = ReadRSSI();

                    // Timestamp the events
                    measurements_.timeUsPacketRxOnChip          = measurements_.timeUsLastISR;
                    measurements_.timeUsPacketRxHandoffComplete = PAL.Micros();

                    // on message received callback
                    rxCb_(_buf, fifoLen);
                }

                // Reset timeout after a clear sign the module is functioning
                signsOfLife_ = 1;
            }
        }
    }

    bool clearRxFifo()
    {
        uint8_t fifo_clear[] = { 0x02 };
        return command(RH_RF24_CMD_FIFO_INFO, fifo_clear, sizeof(fifo_clear));
    }

    uint8_t ReadRSSI()
    {
        uint8_t modem_status[6];
        command(RH_RF24_CMD_GET_MODEM_STATUS, NULL, 0, modem_status, sizeof(modem_status));
    
        return modem_status[3];
    }

    uint8_t IsOperable()
    {
        uint8_t retVal = 1;

        uint8_t buf[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        if (!command(RH_RF24_CMD_PART_INFO, 0, 0, buf, sizeof(buf)))
        {
            retVal = 0;
        }
        else
        {
            uint16_t deviceType = (buf[1] << 8) | buf[2];

            if (deviceType != 0x4463)
            {
                retVal = 0;
            }
        }

        return retVal;
    }

    uint8_t IsOperableReInitIfNot()
    {
        uint8_t retVal = 0;

        if (IsOperable())
        {
            retVal = 1;
        }
        else
        {
            ++stats_.countErrReInitReqd;

            retVal = ReInit();
        }

        return retVal;
    }

    uint8_t ReadFifoLen()
    {
        uint8_t fifoLen;

        command(RH_RF24_CMD_FIFO_INFO, NULL, 0, (uint8_t *)&fifoLen, 1);
        
        return fifoLen;
    }

    uint8_t sendv(IOVec *iovList, uint8_t iovListLen)
    {
        uint8_t retVal = 0;

        if (IsOperableReInitIfNot())
        {
            if (SendvInternal(iovList, iovListLen))
            {
                retVal = IsOperable();
            }
        }

        return retVal;
    }

    uint8_t SendvInternalAtomic(IOVec *iovList, uint8_t iovListLen)
    {
        uint8_t retVal = 0;

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            retVal = SendvInternal(iovList, iovListLen);
        }

        return retVal;
    }

    uint8_t SendvInternal(IOVec *iovList, uint8_t iovListLen)
    {
        uint8_t retVal = 0;

        // Take measurements
        measurements_.timeUsPacketTxStart = PAL.Micros();
        
        // Check if currently sending, if so, return error.
        //
        // Callers must use async callbacks to know when to send again.
        if (_mode != RHModeTx)
        {
            // Calculate if total buffer size exceeds capacity
            uint16_t bufSizeTotal = 0;

            for (uint8_t i = 0; i < iovListLen; ++i)
            {
                bufSizeTotal += iovList[i].bufLen;
            }

            // Observed that sending a 0 sized packet messes up receivers.
            // Unclear why.  Didn't research deeply enough, just don't send
            // 0-byte packets until you need to and then research it.
            if (bufSizeTotal)
            {
                if (bufSizeTotal <= MAX_PAYLOAD_LEN)
                {
                    retVal = 1;

                    countBytesRxQueued_ = bufSizeTotal;

                    uint8_t len = (uint8_t)bufSizeTotal;

                    // Prevent RX while filling the fifo
                    setModeIdle();
                    cmd_clear_all_interrupts();

                    // Single packet mode, so no fragment sending logic necessary.
                    // Fill FIFO completely now based on data in hand.

                    // First, push the size of the payload to be sent.
                    // This is a 1 byte size
                    writeTxFifo((uint8_t *)&len, 1);

                    // Next push all the payload data
                    for (uint8_t i = 0; i < iovListLen; ++i)
                    {
                        writeTxFifo(iovList[i].buf, iovList[i].bufLen);
                    }

                    // Tell the module the size of the payload data as well
                    SetField2Size(len);

                    // Instruct module to send
                    setModeTx();

                    // Take note of timestamp immediately following handoff to module
                    measurements_.timeUsPacketTxHandoffComplete = PAL.Micros();
                }
                else
                {
                    ++stats_.countErrTxOverflow;
                }
            }
        }
        else
        {
            ++stats_.countErrTxEarly;
        }

        return retVal;
    }

    bool send(uint8_t *data, uint8_t len)
    {
        IOVec iov;

        iov.buf    = data;
        iov.bufLen = len;

        return sendv(&iov, 1);
    }

    // This is different to command() since we must not wait for CTS
    bool writeTxFifo(uint8_t *data, uint8_t len)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // First send the command
        digitalWrite(_slaveSelectPin, LOW);
        _spi.beginTransaction();
        _spi.transfer(RH_RF24_CMD_TX_FIFO_WRITE);
        // Now write any write data
        if (len)
        {
            while (len--)
            _spi.transfer(*data++);
        }
        digitalWrite(_slaveSelectPin, HIGH);
        _spi.endTransaction();
        }
        return true;
    }

    void readRxFifo(uint8_t *buf, uint8_t len)
    {
        // This is different to command() since we dont wait for CTS
        digitalWrite(_slaveSelectPin, LOW);

        _spi.transfer(RH_RF24_CMD_RX_FIFO_READ);

        if (len)
        {
            while (len--)
            {
                *buf++ = _spi.transfer(0);
            }
        }

        digitalWrite(_slaveSelectPin, HIGH);
    }

    void setPreambleLength(uint16_t bytes)
    {
        uint8_t config[] = { (uint8_t)bytes, 0x14, 0x00, 0x00, 
                RH_RF24_PREAMBLE_FIRST_1 | RH_RF24_PREAMBLE_LENGTH_BYTES | RH_RF24_PREAMBLE_STANDARD_1010};
        set_properties(RH_RF24_PROPERTY_PREAMBLE_TX_LENGTH, config, sizeof(config));
    }

    bool setCRCPolynomial(CRCPolynomial polynomial)
    {
        if (polynomial >= CRC_NONE &&
        polynomial <= CRC_Castagnoli)
        {
        // Caution this only has effect if CRCs are enabled
        uint8_t config[] = { (uint8_t)((polynomial & RH_RF24_CRC_MASK) | RH_RF24_CRC_SEED_ALL_1S) };
        return set_properties(RH_RF24_PROPERTY_PKT_CRC_CONFIG, config, sizeof(config));
        }
        else
        return false;
    }

    void setSyncWords(const uint8_t* syncWords, uint8_t len)
    {
        if (len > 4 || len < 1)
        return;
        uint8_t config[] = { (uint8_t)(len-1), 0, 0, 0, 0};
        memcpy(config+1, syncWords, len);
        set_properties(RH_RF24_PROPERTY_SYNC_CONFIG, config, sizeof(config));
    }

    bool setFrequency(float centre, float afcPullInRange)
    {
    (void)afcPullInRange; // Not used
        // See Si446x Data Sheet section 5.3.1
        // Also the Si446x PLL Synthesizer / VCO_CNT Calculator Rev 0.4
        uint8_t outdiv;
        uint8_t band;
        if (_deviceType == 0x4460 ||
        _deviceType == 0x4461 ||
        _deviceType == 0x4463)
        {
        // Non-continuous frequency bands
        if (centre <= 1050.0 && centre >= 850.0)
            outdiv = 4, band = 0;
        else if (centre <= 525.0 && centre >= 425.0)
            outdiv = 8, band = 2;
        else if (centre <= 350.0 && centre >= 284.0)
            outdiv = 12, band = 3;
        else if (centre <= 175.0 && centre >= 142.0)
            outdiv = 24, band = 5;
        else 
            return false;
        }
        else
        {
        // 0x4464
        // Continuous frequency bands
        if (centre <= 960.0 && centre >= 675.0)
            outdiv = 4, band = 1;
        else if (centre < 675.0 && centre >= 450.0)
            outdiv = 6, band = 2;
        else if (centre < 450.0 && centre >= 338.0)
            outdiv = 8, band = 3;
        else if (centre < 338.0 && centre >= 225.0)
            outdiv = 12, band = 4;
        else if (centre < 225.0 && centre >= 169.0)
            outdiv = 16, band = 4;
        else if (centre < 169.0 && centre >= 119.0)
            outdiv = 24, band = 5;
        else 
            return false;
        }

        // Set the MODEM_CLKGEN_BAND (not documented)
        uint8_t modem_clkgen[] = { (uint8_t)(band + 8) };
        if (!set_properties(RH_RF24_PROPERTY_MODEM_CLKGEN_BAND, modem_clkgen, sizeof(modem_clkgen)))
        return false;

        centre *= 1000000.0; // Convert to Hz

        // Now generate the RF frequency properties
        // Need the Xtal/XO freq from the radio_config file:
        uint32_t xtal_frequency = RADIO_CONFIGURATION_DATA_RADIO_XO_FREQ;
        unsigned long f_pfd = 2 * xtal_frequency / outdiv;
        unsigned int n = ((unsigned int)(centre / f_pfd)) - 1;
        float ratio = centre / (float)f_pfd;
        float rest  = ratio - (float)n;
        unsigned long m = (unsigned long)(rest * 524288UL); 
        unsigned int m2 = m / 0x10000;
        unsigned int m1 = (m - m2 * 0x10000) / 0x100;
        unsigned int m0 = (m - m2 * 0x10000 - m1 * 0x100); 

        // PROP_FREQ_CONTROL_GROUP
        uint8_t freq_control[] = { (uint8_t)n, (uint8_t)m2, (uint8_t)m1, (uint8_t)m0 };
        return set_properties(RH_RF24_PROPERTY_FREQ_CONTROL_INTE, freq_control, sizeof(freq_control));
    }

    void setModeIdle()
    {
        if (_mode != RHModeIdle)
        {
        // Set the antenna switch pins using the GPIO, assuming we have an RFM module with antenna switch
        uint8_t config[] = { RH_RF24_GPIO_HIGH, RH_RF24_GPIO_HIGH };
        command(RH_RF24_CMD_GPIO_PIN_CFG, config, sizeof(config));

        uint8_t state[] = { _idleMode };
        command(RH_RF24_CMD_CHANGE_STATE, state, sizeof(state));
        _mode = RHModeIdle;
        }
    }

    bool sleep()
    {
        if (_mode != RHModeSleep)
        {
            // This will change to SLEEP or STANDBY, depending on the value of GLOBAL_CLK_CFG:CLK_32K_SEL.
            // which default to 0, eg STANDBY
        uint8_t state[] = { RH_RF24_DEVICE_STATE_SLEEP };
        command(RH_RF24_CMD_CHANGE_STATE, state, sizeof(state));

        _mode = RHModeSleep;
        }
        return true;
    }

    void SetField2MaxSize()
    {
        // This is a 13-bit unsigned integer
        SetField2Size(0x1FFF);
    }

    void SetField2Size(uint16_t size)
    {
        // This is a 13-bit unsigned integer
        const uint16_t sizeBigEndian = PAL.htons(size);

        set_properties(RH_RF24_PROPERTY_PKT_FIELD_2_LENGTH_12_8, (uint8_t *)&sizeBigEndian, 2);
    }

    void setModeRx()
    {
        SetModeRxInternal();
    }

    void SetModeRxInternal()
    {
        // Tell the receiver the max data length we will accept, which is the maximum which can
        // fit into the FIFO.
        // We want all packets to be received and processed in the application.
        // Despite the fact that we're not able to receive the full-sized packet, we want
        // to know that.
        //
        // Observations show that packets received greater than this configured limit
        // show up as a CRC error interrupt.
        //
        // Following this change, any sized packet should be observable (though not received).
        //
        SetField2MaxSize();
        
        // Set the antenna switch pins using the GPIO, assuming we have an RFM module with antenna switch
        uint8_t gpio_config[] = { RH_RF24_GPIO_HIGH, RH_RF24_GPIO_LOW };
        command(RH_RF24_CMD_GPIO_PIN_CFG, gpio_config, sizeof(gpio_config));

        uint8_t rx_config[] = { 0x00, RH_RF24_CONDITION_RX_START_IMMEDIATE, 0x00, 0x00, _idleMode, _idleMode, _idleMode};
        command(RH_RF24_CMD_START_RX, rx_config, sizeof(rx_config));
        _mode = RHModeRx;
    }

    void setModeTx()
    {
        if (_mode != RHModeTx)
        {
        // Set the antenna switch pins using the GPIO, assuming we have an RFM module with antenna switch
        uint8_t config[] = { RH_RF24_GPIO_LOW, RH_RF24_GPIO_HIGH };
        command(RH_RF24_CMD_GPIO_PIN_CFG, config, sizeof(config));

        uint8_t tx_params[] = { 0x00, 
                    (uint8_t)((_idleMode << 4) | RH_RF24_CONDITION_RETRANSMIT_NO | RH_RF24_CONDITION_START_IMMEDIATE)};
        command(RH_RF24_CMD_START_TX, tx_params, sizeof(tx_params));
        
        // don't change internal mode unless actually successfully changed the module mode.
        // this state will be used later to decide if to artifically fire a ontransmit callback.
        if (IsOperable())
        {
            _mode = RHModeTx;
        }

        }
    }

    // chip defaults to maximum power.
    // this function allows tuning 0-127.
    void setTxPower(uint8_t power)
    {
        txPower_ = power;

        uint8_t pa_bias_clkduty = 0;
        // These calculations valid for advertised power from Si chips at Vcc = 3.3V
        // you may get lower power from RFM modules, depending on Vcc voltage, antenna etc

        // 0x7f = 20dBm
        pa_bias_clkduty = 0x00; // Per WDS suggestion
        if (power > 0x7f)
            power = 0x7f;

        uint8_t power_properties[] = {0x08, 0x00, 0x00 }; // PA_MODE from WDS sugggestions (why?)
        power_properties[1] = power;
        power_properties[2] = pa_bias_clkduty;
        set_properties(RH_RF24_PROPERTY_PA_MODE, power_properties, sizeof(power_properties));
    }

    // Caution: There was a bug in A1 hardware that will not handle 1 byte commands. 
    bool command(uint8_t cmd, const uint8_t* write_buf = 0, uint8_t write_len = 0, uint8_t* read_buf = 0, uint8_t read_len = 0)
    {
        bool   done = false;

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // First send the command
        digitalWrite(_slaveSelectPin, LOW);
        _spi.transfer(cmd);

        // Now write any write data
        if (write_buf && write_len)
        {
        while (write_len--)
            _spi.transfer(*write_buf++);
        }
        // Sigh, the RFM26 at least has problems if we deselect too quickly :-(
        // Innocuous timewaster:
        digitalWrite(_slaveSelectPin, LOW);
        // And finalise the command
        digitalWrite(_slaveSelectPin, HIGH);

        uint16_t count; // Number of times we have tried to get CTS
        for (count = 0; !done && count < RH_RF24_CTS_RETRIES; count++)
        {
        // Wait for the CTS
        digitalWrite(_slaveSelectPin, LOW);

        _spi.transfer(RH_RF24_CMD_READ_BUF);
        if (_spi.transfer(0) == RH_RF24_REPLY_CTS)
        {
            // Now read any expected reply data
            if (read_buf && read_len)
            {
            while (read_len--)
                *read_buf++ = _spi.transfer(0);
            }
            done = true;
        }
        // Sigh, the RFM26 at least has problems if we deselect too quickly :-(
        // Innocuous timewaster:
        digitalWrite(_slaveSelectPin, LOW);
        // Finalise the read
        digitalWrite(_slaveSelectPin, HIGH);
        }
        }
        return done; // False if too many attempts at CTS
    }

    bool configure(const uint8_t* commands)
    {
        // Command strings are constructed in radio_config_Si4460.h 
        // Each command starts with a count of the bytes in that command:
        // <bytecount> <command> <bytecount-2 bytes of args/data>
        uint8_t next_cmd_len;
        
        while (memcpy_P(&next_cmd_len, commands, 1), next_cmd_len > 0)
        {
        uint8_t buf[20]; // As least big as the biggest permitted command/property list of 15
        memcpy_P(buf, commands+1, next_cmd_len);
        command(buf[0], buf+1, next_cmd_len - 1);
        commands += (next_cmd_len + 1);
        }
        return true;
    }

    void power_on_reset()
    {
        // Sigh: its necessary to control the SDN pin to reset this ship. 
        // Tying it to GND does not produce reliable startups
        // Per Si4464 Data Sheet 3.3.2
        digitalWrite(_sdnPin, HIGH); // So we dont get a glitch after setting pinMode OUTPUT
        pinMode(_sdnPin, OUTPUT);
        delay(10);
        digitalWrite(_sdnPin, LOW);
        delay(10);
    }

    bool cmd_clear_all_interrupts()
    {
        uint8_t write_buf[] = { 0x00, 0x00, 0x00 }; 
        return command(RH_RF24_CMD_GET_INT_STATUS, write_buf, sizeof(write_buf));
    }

    bool set_properties(uint16_t firstProperty, const uint8_t* values, uint8_t count)
    {
        uint8_t buf[15];

        buf[0] = firstProperty >> 8;   // GROUP
        buf[1] = count;                // NUM_PROPS
        buf[2] = firstProperty & 0xff; // START_PROP
        uint8_t i;
        for (i = 0; i < 12 && i < count; i++)
        buf[3 + i] = values[i]; // DATAn
        return command(RH_RF24_CMD_SET_PROPERTY, buf, count + 3);
    }

    bool get_properties(uint16_t firstProperty, uint8_t* values, uint8_t count)
    {
        if (count > 16)
        count = 16;
        uint8_t buf[3];
        buf[0] = firstProperty >> 8;   // GROUP
        buf[1] = count;                // NUM_PROPS
        buf[2] = firstProperty & 0xff; // START_PROP
        return command(RH_RF24_CMD_GET_PROPERTY, buf, sizeof(buf), values, count);
    }

    float get_temperature()
    {
        uint8_t write_buf[] = { 0x10 };
        uint8_t read_buf[8];
        // Takes nearly 4ms
        command(RH_RF24_CMD_GET_ADC_READING, write_buf, sizeof(write_buf), read_buf, sizeof(read_buf));
        uint16_t temp_adc = (read_buf[4] << 8) | read_buf[5];
        return ((800 + read_buf[6]) / 4096.0) * temp_adc - ((read_buf[7] / 2) + 256);
    }

    float get_battery_voltage()
    {
        uint8_t write_buf[] = { 0x08 };
        uint8_t read_buf[8];
        // Takes nearly 4ms
        command(RH_RF24_CMD_GET_ADC_READING, write_buf, sizeof(write_buf), read_buf, sizeof(read_buf));
        uint16_t battery_adc = (read_buf[2] << 8) | read_buf[3];
        return 3.0 * battery_adc / 1280;
    }

    float get_gpio_voltage(uint8_t gpio)
    {
        uint8_t write_buf[] = { 0x04 };
        uint8_t read_buf[8];
        write_buf[0] |= (gpio & 0x3);
        // Takes nearly 4ms
        command(RH_RF24_CMD_GET_ADC_READING, write_buf, sizeof(write_buf), read_buf, sizeof(read_buf));
        uint16_t gpio_adc = (read_buf[0] << 8) | read_buf[1];
        return 3.0 * gpio_adc / 1280;
    }

    uint8_t frr_read(uint8_t reg)
    {
        uint8_t ret;

        // Do not wait for CTS
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // First send the command
        digitalWrite(_slaveSelectPin, LOW);
        _spi.transfer(RH_RF24_PROPERTY_FRR_CTL_A_MODE + reg);
        // Get the fast response
        ret = _spi.transfer(0);
        digitalWrite(_slaveSelectPin, HIGH);
        }
        return ret;
    }

    uint16_t deviceType() {return _deviceType;};
    uint16_t _deviceType = 0;


    /// The configured pin connected to the SDN pin of the radio
    uint8_t             _sdnPin;

    /// The radio OP mode to use when mode is RHModeIdle
    uint8_t             _idleMode; 

    /// Array of octets of the last received message or the next to transmit message
    uint8_t             _buf[MAX_PAYLOAD_LEN];


    RHMode     _mode;

    uint8_t txPower_ = 127;

    struct Measurements
    {
        uint32_t timeUsLastISR  = 0; // Last ISR time deep within Ivm

        // Receive timestamps
        // First there's a HW interrupt from the chip saying message received
        // Second there's extracting from chip, processing, ready to fire callback
        //
        // There is no asynchronous state between these two, they're synchronously in order
        //
        uint32_t timeUsPacketRxOnChip          = 0; // ISR time, later identified as being chip RX packet received interrupt
        uint32_t timeUsPacketRxHandoffComplete = 0; // After RxOnChip, immediately before callback

        // Transmit timestamps
        //
        // First two are part of async send.
        //   There is no async state between the two, they're synchronously in order
        // Third is notification the chip has completed sending
        //   This is an async callback
        uint32_t timeUsPacketTxStart           = 0; // Immediately within send()
        uint32_t timeUsPacketTxHandoffComplete = 0; // After TxStart, after SPI transfer of data to chip
        uint32_t timeUsPacketTxComplete        = 0; // ISR time, later identified as being chip TX complete interrupt

        // Received signal strength indicator
        uint8_t rssi = 0;
    };

    Measurements GetMeasurements() const
    {
        return measurements_;
    }

    Measurements measurements_;


    struct Stats
    {
        uint16_t countErrReInitReqd  = 0;
        uint16_t countErrInvalidSync = 0;
        uint16_t countErrCrc         = 0;
        uint16_t countErrRxOverflow  = 0;
        uint16_t countErrTxOverflow  = 0;
        uint16_t countErrTxEarly     = 0;

        uint16_t countIrqModem = 0;
        uint16_t countIrqPh    = 0;

        uint16_t countPacketsRx = 0;
        uint16_t countBytesRx   = 0;

        uint16_t countPacketsTx = 0;
        uint16_t countBytesTx   = 0;
    };

    Stats stats_;
    uint8_t countBytesRxQueued_ = 0;

    Stats GetStats() const
    {
        return stats_;
    }


    InterruptEventHandlerDelegate ied_;

    void SetOnMessageReceivedCallback(function<void(uint8_t *buf, uint8_t bufSize)> rxCb)
    {
        rxCb_ = rxCb;
    }

    void SetOnMessageTransmittedCallback(function<void()> txCb)
    {
        txCb_ = txCb;
    }
    
    function<void()>                             txCb_;
    function<void(uint8_t *buf, uint8_t bufLen)> rxCb_;

    TimedEventHandlerDelegate ted_;
    uint8_t                   signsOfLife_ = 0;
};





#endif
