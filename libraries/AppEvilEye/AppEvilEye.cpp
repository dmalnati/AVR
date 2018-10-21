#include "AppEvilEye.h"


const uint8_t AppEvilEye::ROW_PIN_LIST[AppEvilEye::ROW_COUNT] = { 14, 15, 16, 17, 18, 19, 9, 10, 23, 24, 25, 26, 4, 5 };
const uint8_t AppEvilEye::COL_PIN_LIST[AppEvilEye::COL_COUNT] = { 6, 11, 12, 13 };

const uint8_t AppEvilEye::IRIS_PIN_LIST[AppEvilEye::IRIS_PIN_LIST_LEN] = { 4, 0, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52 };

const uint8_t AppEvilEye::BORDER_UPPER_RIGHT_PIN_LIST[AppEvilEye::BORDER_PIN_LIST_LEN] = {  1,  9, 25, 41, 10 };
const uint8_t AppEvilEye::BORDER_LOWER_RIGHT_PIN_LIST[AppEvilEye::BORDER_PIN_LIST_LEN] = { 42, 11, 27, 43, 51 };
const uint8_t AppEvilEye::BORDER_LOWER_LEFT_PIN_LIST[AppEvilEye::BORDER_PIN_LIST_LEN]  = { 55, 47, 31, 15, 46 };
const uint8_t AppEvilEye::BORDER_UPPER_LEFT_PIN_LIST[AppEvilEye::BORDER_PIN_LIST_LEN]  = { 14, 45, 29, 13,  5 };
    
const uint8_t *AppEvilEye::BORDER_PIN_LIST_LIST[AppEvilEye::BORDER_PIN_LIST_LIST_LEN] = {
    AppEvilEye::BORDER_UPPER_RIGHT_PIN_LIST,
    AppEvilEye::BORDER_LOWER_RIGHT_PIN_LIST,
    AppEvilEye::BORDER_LOWER_LEFT_PIN_LIST,
    AppEvilEye::BORDER_UPPER_LEFT_PIN_LIST,
};

    
    