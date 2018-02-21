#ifndef __STREAM_WINDOW_H__
#define __STREAM_WINDOW_H__


/*
 * Design objectives
 * - constantly add elements
 * - old ones get pushed out as buffer fills
 * - uses someone else's buffer
 * 
 */

template <typename T>
class StreamWindow
{
public:
    StreamWindow()
    {
        Attach(NULL, 0);
    }
    
    StreamWindow(T *arr, uint8_t elementCount, T filler = T{})
    {
        Attach(arr, elementCount, filler);
    }
    
    void Attach(T *arr, uint8_t elementCount, T filler = T{})
    {
        arr_          = NULL;
        arrSize_      = 0;
        elementCount_ = 0;
        filler        = T{};
        
        if (arr && elementCount)
        {
            arr_          = arr;
            arrSize_      = 0;
            elementCount_ = elementCount;
            filler_       = filler;
            
            Reset();
        }
    }
    
    void Reset()
    {
        if (arr_)
        {
            for (uint8_t i = 0; i < elementCount_; ++i)
            {
                arr_[i] = filler_;
            }
            
            arrSize_ = 0;
        }
    }
    
    uint8_t Append(T &val)
    {
        uint8_t retVal = 0;
        
        if (arr_)
        {
            retVal = MakeRoomForOneMore();
            
            if (retVal)
            {
                arr_[arrSize_] = val;
                ++arrSize_;
            }
        }
        
        return retVal;
    }
    
    uint8_t Size()
    {
        uint8_t retVal = 0;
        
        if (arr_)
        {
            retVal = arrSize_;
        }
        
        return retVal;
    }
    
    uint8_t Capacity()
    {
        uint8_t retVal = 0;
        
        if (arr_)
        {
            retVal = elementCount_;
        }
    
        return retVal;
    }
    

private:
    
    uint8_t MakeRoomForOneMore()
    {
        uint8_t retVal = 0;
        
        if (arr_)
        {
            retVal = 1;
            
            if (arrSize_ == elementCount_)
            {
                // out of space, shift old out
                for (uint8_t i = 0; i < elementCount_; ++i)
                {
                    arr_[i] = arr_[i + 1];
                }
                
                // update buffer size to be one less for shifted-out element
                --arrSize_;
                
                // fill in hole
                arr_[arrSize_] = filler_;
            }
        }
        
        return retVal;
    }


    T       *arr_;
    uint8_t  arrSize_;
    uint8_t  elementCount_;
    T        filler_;
};


#endif  // __STREAM_WINDOW_H__








































