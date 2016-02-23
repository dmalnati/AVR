#ifndef __CONTAINER_H__
#define __CONTAINER_H__


#include <util/atomic.h>

#include <stdint.h>
#include <stdlib.h>

 
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
 

 
 
template<typename T, uint8_t CAPACITY>
class RingBuffer
{
private:
    static const uint8_t OVERFLOW_BOUNDARY = (uint8_t)-1;
    
public:
    RingBuffer()
    : idxFront_(0)
    , idxBack_(0)
    , size_(0)
    , capacity_(CAPACITY ? CAPACITY : 1)
    {
        // Nothing to do
    }
    
    virtual ~RingBuffer()
    {
        // Nothing to do
    }
 
    /////////// Basic Getters / Setters ///////////
    
    uint8_t Capacity() const { return capacity_; }
    uint8_t Size() const { return size_; }
    
    
    /////////// Front Access ///////////
    
    uint8_t PushFront(T element)
    {
        uint8_t retVal = CanFitOneMore();
        
        if (retVal)
        {
            --idxFront_;
            if (idxFront_ >= capacity_) { idxFront_ = (capacity_ - 1); }
            
            table_[idxFront_] = element;
            
            ++size_;
        }
        
        return retVal;
    }
    
    uint8_t PeekFront(T &element) const
    {
        uint8_t retVal = 0;
        
        if (size_)
        {
            retVal = 1;
            
            element = table_[idxFront_];
        }
        
        return retVal;
    }
 
    uint8_t PopFront(T &element)
    {
        uint8_t retVal = 0;
        
        if (size_)
        {
            retVal = 1;
            
            element = table_[idxFront_];
     
            ++idxFront_;
            if (idxFront_ >= capacity_) { idxFront_ = 0; }
     
            --size_;
        }
        
        return retVal;
    }
    
    
    /////////// Back Access ///////////
 
    uint8_t PushBack(T element)
    {
        uint8_t retVal = CanFitOneMore();
        
        if (retVal)
        {
            table_[idxBack_] = element;
     
            ++idxBack_;
            if (idxBack_ >= capacity_) { idxBack_ = 0; }
     
            ++size_;
        }
        
        return retVal;
    }
    
    uint8_t PopBack(T &element)
    {
        uint8_t retVal = 0;
        
        if (size_)
        {
            retVal = 1;
            
            --idxBack_;
            if (idxBack_ >= capacity_) { idxBack_ = (capacity_ - 1); }
            
            element = table_[idxBack_];
            
            --size_;
        }
        
        return retVal;
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
    
    uint8_t PopAt(uint8_t idxLogical, T &element)
    {
        uint8_t retVal = 0;
        
        if (idxLogical < size_)
        {
            retVal = 1;
            
            // Hold a temp copy to return after array shifted
            element = (*this)[idxLogical];
            
            // Calculate elements to the right of this element
            // We shift all elements, then call PopBack to update
            // data structure internals.
            uint8_t elementsToShift = (Size() - idxLogical) - 1;
            
            // Shift everything remaining to the left
            for (uint8_t i = 0; i < elementsToShift; ++i)
            {
                (*this)[idxLogical + i] = (*this)[idxLogical + i + 1];
            }
            
            T elementTmp;
            PopBack(elementTmp);
        }
        
        return retVal;
    }
    
    uint8_t Remove(T element)
    {
        uint8_t retVal     = 0;
        uint8_t idxLogical = 0;
        
        T elementTmp;
        
        if (FindIdxFirst(element, idxLogical))
        {
            retVal = PopAt(idxLogical, elementTmp);
        }
        
        return retVal;
    }
    
    
    /////////// Search ///////////
    
    uint8_t FindIdxFirst(T element, uint8_t &idxLogical)
    {
        uint8_t found = 0;
        uint8_t size  = Size();
        
        for (idxLogical = 0; idxLogical < size; ++idxLogical)
        {
            if (element == (*this)[idxLogical])
            {
                found = 1;
                break;
            }
        }
        
        return found;
    }
    
    
    
    
    /////////// Debug ///////////
    
#ifdef DEBUG

    void PrintDebug(const char *msg)
    {
        printf("%s:\n", msg);
        printf("capacity_ : %i\n", capacity_);
        printf("size_    : %i\n", size_);
        printf("idxFront_: %i\n", idxFront_);
        printf("idxBack_ : %i\n", idxBack_);
        printf("Logical Elements:\n");
        for (uint8_t i = 0; i < Size(); ++i)
        {
            printf("    %3i: %i\n", i, (uint32_t)(*this)[i]);
        }
        printf("Actual Elements:\n");
        for (uint8_t i = 0; i < Capacity(); ++i)
        {
            printf("    %3i: %i\n", i, (uint32_t)table_[i]);
        }
        printf("\n");
    }

#endif

   
private:
    uint8_t CanFitOneMore()
    {
        return (size_ != capacity_);
    }

    
    uint8_t idxFront_;
    uint8_t idxBack_;
    uint8_t size_;
    uint8_t capacity_;
 
    T table_[CAPACITY ? CAPACITY : 1];
};



template<typename T, uint8_t CAPACITY>
class Queue
: protected RingBuffer<T, CAPACITY>
{
public:
    Queue()
    : RingBuffer<T, CAPACITY>()
    {
        // nothing to do
    }
    
    // virtual so SortedQueue can implement same function
    virtual uint8_t Push(T element)
    {
        return RingBuffer<T, CAPACITY>::PushBack(element);
    }
    
    uint8_t Pop(T &element)
    {
        return RingBuffer<T, CAPACITY>::PopFront(element);
    }
    
    uint8_t Peek(T &element)
    {
        return RingBuffer<T, CAPACITY>::PeekFront(element);
    }
    
    uint8_t Size()
    {
        return RingBuffer<T, CAPACITY>::Size();
    }
    
    uint8_t HasElement(T element)
    {
        uint8_t retVal      = 0;
        uint8_t tmpRetParam;
        
        if (RingBuffer<T, CAPACITY>::FindIdxFirst(element, tmpRetParam))
        {
            retVal = 1;
        }
        
        return retVal;
    }
    
    uint8_t Remove(T element)
    {
        return RingBuffer<T, CAPACITY>::Remove(element);
    }
    
    T &operator[](uint8_t idxLogical)
    {
        return RingBuffer<T, CAPACITY>::operator[](idxLogical);
    }
    
    
    /////////// Debug ///////////
    
#ifdef DEBUG

    void PrintDebug(const char *msg)
    {
        RingBuffer<T, CAPACITY>::PrintDebug(msg);
    }
    
#endif // DEBUG

};


template<typename T, uint8_t CAPACITY, typename CMP>
class SortedQueue
: public Queue<T, CAPACITY>
{
public:
    SortedQueue()
    : Queue<T, CAPACITY>()
    {
        // nothing to do
    }
    
    // inserts before the first element greater than it.
    // only applies to this element.
    // for a sorted list, strictly use this function from start to finish.
    virtual uint8_t Push(T element)
    {
        uint8_t retVal = 0;
        
        // First, simply insert the element at the front.
        if (RingBuffer<T, CAPACITY>::PushFront(element))
        {
            retVal = 1;
            
            // Instantiate comparator
            CMP cmp;
            
            // Only try to sort if there are more than one elements
            uint8_t size = RingBuffer<T, CAPACITY>::Size();
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
        
        return retVal;
    }
};







#endif  // __CONTAINER_H__











































