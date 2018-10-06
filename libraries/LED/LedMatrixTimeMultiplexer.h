#ifndef __LED_MATRIX_TIME_MULTIPLEXER_H__
#define __LED_MATRIX_TIME_MULTIPLEXER_H__


#include "PAL.h"


template <uint8_t ROW_COUNT, uint8_t COL_COUNT>
class LedMatrixTimeMultiplexer
{
private:

    static const uint32_t DEFAULT_ROW_INTERVAL_MS = 1;

    struct LedData
    {
        uint8_t onOff = 0;
    };


public:

    LedMatrixTimeMultiplexer(const uint8_t (&& rowPinList)[ROW_COUNT],
                             const uint8_t (&& colPinList)[COL_COUNT])
    : LedMatrixTimeMultiplexer(rowPinList, colPinList)
    {
        // Nothing to do
    }
    
    LedMatrixTimeMultiplexer(const uint8_t (& rowPinList)[ROW_COUNT],
                             const uint8_t (& colPinList)[COL_COUNT])
    {
        // Store, set all pins as output, set low
        for (uint8_t row = 0; row < ROW_COUNT; ++row)
        {
            rowPinList_[row] = rowPinList[row];
            
            PAL.PinMode(rowPinList_[row], OUTPUT);
            PAL.DigitalWrite(rowPinList_[row], LOW);
        }
        
        for (uint8_t col = 0; col < COL_COUNT; ++col)
        {
            colPinList_[col] = colPinList[col];
            
            PAL.PinMode(colPinList_[col], OUTPUT);
            PAL.DigitalWrite(colPinList_[col], LOW);
        }
    }
    
    void SetLedState(uint8_t row, uint8_t col, uint8_t onOff)
    {
        if (row < ROW_COUNT && col < COL_COUNT)
        {
            rowByColMatrix_[row][col].onOff = onOff;
        }
    }
    
    uint8_t GetLedState(uint8_t row, uint8_t col)
    {
        uint8_t retVal = 0;
        
        if (row < ROW_COUNT && col < COL_COUNT)
        {
            retVal = rowByColMatrix_[row][col].onOff;
        }
        
        return retVal;
    }
    
    void SetRowIntervalMs(uint32_t rowIntervalMs)
    {
        rowIntervalMs_ = rowIntervalMs;
    }

    void Start()
    {
        Stop();
        
        ted_.SetCallback([this](){
            RenderNextRow();
        });
        
        ted_.RegisterForTimedEventInterval(rowIntervalMs_);
    }
    
    void Stop()
    {
        RowClear(rowIdx_);
        
        rowIdx_ = 0;
        
        ted_.DeRegisterForTimedEvent();
    }


private:
    
    void RenderNextRow()
    {
        RowClear(rowIdx_);
        
        rowIdx_ = (rowIdx_ + 1);
        if (rowIdx_ == ROW_COUNT)
        {
            rowIdx_ = 0;
        }
        
        RowPaint(rowIdx_);
    }
    
    void RowPaint(uint8_t rowIdx)
    {
        // Activate the row
        PAL.DigitalWrite(rowPinList_[rowIdx], HIGH);
        
        for (uint8_t col = 0; col < COL_COUNT; ++col)
        {
            LedData &ledData = rowByColMatrix_[rowIdx][col];
            
            // Activate the column if the LED is set on
            if (ledData.onOff)
            {
                PAL.DigitalWrite(colPinList_[col], HIGH);
            }
        }
    }
    
    void RowClear(uint8_t rowIdx)
    {
        // Deactivate the row
        PAL.DigitalWrite(rowPinList_[rowIdx], LOW);
        
        // Unconditionally put all low.
        // Deals with scenario where between RowPaint and RowClear, the current
        // row is modified by calling code and therefore we would lose the
        // ability to see what's set by looking at stateful data.
        // Here no state is required.
        for (uint8_t col = 0; col < COL_COUNT; ++col)
        {
            PAL.DigitalWrite(colPinList_[col], LOW);
        }
    }



    Pin rowPinList_[ROW_COUNT];
    Pin colPinList_[COL_COUNT];

    LedData rowByColMatrix_[ROW_COUNT][COL_COUNT];
    
    uint8_t rowIdx_ = 0;
    
    uint32_t                   rowIntervalMs_ = DEFAULT_ROW_INTERVAL_MS;
    TimedEventHandlerDelegate  ted_;
    
};
























#endif  // __LED_MATRIX_TIME_MULTIPLEXER_H__