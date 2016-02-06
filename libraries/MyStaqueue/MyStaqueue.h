#ifndef __MY_STAQUEUE_H__
#define __MY_STAQUEUE_H__


#include <stdint.h>

 
//
// Minimum valid: 1   elements (0th   index)
// Maximum valid: 255 elements (254th index)
//
//
// Common operations, order of frequency:
// - iterate
// - peek front
// - pop front
// - insert sorted
// - push back
 
//
// State Keeping
//
// idxFront_ points to the actual first element.  Fast access.
// idxBack_  points to the next free slot.
 

 
 
template<typename T>
class MyStaqueue
{  
public:
    MyStaqueue(uint8_t capacity)
    : capacity_(capacity)
    , idxFront_(0)
    , idxBack_(0)
    , size_(0)
    {
        table_ = new T[capacity_];
    }
    ~MyStaqueue()
    {
        delete[] table_;
    }
 
    /////////// Basic Getters / Setters ///////////
    
    uint8_t Capacity() const { return capacity_; }
    uint8_t Size() const { return size_; }
    
    
    /////////// Front Access ///////////
    
    void PushFront(T element)
    {
        --idxFront_;
        if (idxFront_ >= capacity_) { idxFront_ = (capacity_ - 1); }
        
        table_[idxFront_] = element;
        
        ++size_;
    }
    
    T PeekFront() const { return table_[idxFront_]; }
 
    T PopFront()
    {
        T element = table_[idxFront_];
 
        ++idxFront_;
        if (idxFront_ >= capacity_) { idxFront_ = 0; }
 
        --size_;
 
        return element;
    }
    
    
    /////////// Back Access ///////////
 
    void PushBack(T element)
    {
        table_[idxBack_] = element;
 
        ++idxBack_;
        if (idxBack_ >= capacity_) { idxBack_ = 0; }
 
        ++size_;
    }
    
    T PopBack()
    {
        --idxBack_;
        if (idxBack_ >= capacity_) { idxBack_ = (capacity_ - 1); }
        
        T element = table_[idxBack_];
        
        --size_;
        
        return element;
    }
    
    
    /////////// Random Access ///////////
   
    T &operator[](uint8_t idxLogical)
    {
        // take best initial shot at correct actual index
        uint8_t idxActual = idxFront_ + idxLogical;
 
        if (idxActual < idxLogical)
        {
            // actual integer wrap
            idxActual += (OVERFLOW_BOUNDARY - capacity_) + 1;
        }
        else if (idxActual >= capacity_)
        {
            // logical wrap
            idxActual -= capacity_;
        }
   
        return table_[idxActual];
    }
    
    T PopAt(uint8_t idxLogical)
    {
        // Hold a temp copy to return after array shifted
        T element = (*this)[idxLogical];
        
        // Calculate elements to the right of this element
        // We shift all elements, then call PopBack to update
        // data structure internals.
        uint8_t elementsToShift = (Size() - idxLogical) - 1;
        
        // Shift everything remaining to the left
        for (uint8_t i = idxLogical; i <= elementsToShift; ++i)
        {
            (*this)[i] = (*this)[i + 1];
        }
        
        PopBack();
        
        return element;
    }
    
    bool Remove(T element)
    {
        bool    retVal     = false;
        uint8_t idxLogical = 0;
        
        if (FindIdxFirst(element, idxLogical))
        {
            PopAt(idxLogical);
            
            retVal = true;
        }
        
        return retVal;
    }
    
    
    /////////// Search ///////////
    
    bool FindIdxFirst(T element, uint8_t &idxLogical)
    {
        bool    found = false;
        uint8_t size  = Size();
        
        for (idxLogical = 0; idxLogical < size; ++idxLogical)
        {
            if (element == (*this)[idxLogical])
            {
                found = true;
                break;
            }
        }
        
        return found;
    }
    
    
    /////////// Sorted Access ///////////
 
    typedef int8_t (*CmpFn)(T left, T right);
    
    // inserts before the first element greater than it.
    // only applies to this element.
    // for a sorted list, strictly use this function from start to finish.
    void PushSorted(T element, CmpFn cmp)
    {
        // First, simply insert the element at the front.
        PushFront(element);
        
        // Only try to sort if there are more than one elements
        uint8_t size = Size();
        if (size > 1)
        {
            bool keepGoing = true;
            
            for (uint8_t idxFirst = 0, idxSecond = 1;
                 idxSecond < size && keepGoing;
                 ++idxFirst, ++idxSecond)
            {
                T valFirst  = (*this)[idxFirst];
                T valSecond = (*this)[idxSecond];
                
                int8_t cmpVal = cmp(valFirst, valSecond);
                
                if (cmpVal < 0)
                {
                    // stop here
                    // element to the right is larger, so no swap needed
                    keepGoing = false;
                }
                else // (cmpVal >= 0)
                {
                    // element to the right is lower, swap places
                    
                    (*this)[idxFirst]  = valSecond;
                    (*this)[idxSecond] = valFirst;
                }
            }
        }
    }
    
    
    /////////// Debug ///////////
    
#ifdef DEBUG

    void PrintDebug()
    {
        printf("idxFront_: %i\n", idxFront_);
        printf("idxBack_ : %i\n", idxBack_);
        printf("size_    : %i\n", size_);
        printf("Elements :\n");
        for (uint8_t i = 0; i < Size(); ++i)
        {
            printf("    %3i: %i\n", i, (uint32_t)(*this)[i]);
        }
    }

#endif
   
   
private:
    static const uint8_t OVERFLOW_BOUNDARY = (uint8_t)-1;
 
    uint8_t capacity_;
    uint8_t idxFront_;
    uint8_t idxBack_;
    uint8_t size_;
 
    T *table_;
};




#endif  // __MY_STAQUEUE_H__




