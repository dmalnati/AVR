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
 

 
 
template<typename T>
class RingBuffer
{
private:
    static const uint8_t OVERFLOW_BOUNDARY = (uint8_t)-1;
    
public:
    static const uint8_t RING_BUFFER_DEFAULT_CAPACITY  = 4;
    static const uint8_t RING_BUFFER_DEFAULT_GROW_SIZE = 4;

    RingBuffer(uint8_t capacity = RING_BUFFER_DEFAULT_CAPACITY,
               uint8_t growSize = RING_BUFFER_DEFAULT_GROW_SIZE)
    : capacity_(capacity)
    , growSize_(growSize)
    , idxFront_(0)
    , idxBack_(0)
    , size_(0)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            table_ = (T *)malloc(sizeof(T) * capacity_);
        }
    }
    virtual ~RingBuffer()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            free(table_);
        }
    }
 
    /////////// Basic Getters / Setters ///////////
    
    uint8_t Capacity() const { return capacity_; }
    uint8_t Size() const { return size_; }
    
    
    /////////// Front Access ///////////
    
    uint8_t PushFront(T element)
    {
        uint8_t retVal = EnsureCapacityForOneMore();
        
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
        uint8_t retVal = EnsureCapacityForOneMore();
        
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
            for (uint8_t i = idxLogical; i <= elementsToShift; ++i)
            {
                (*this)[i] = (*this)[i + 1];
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
        printf("capacity_: %i\n", capacity_);
        printf("growSize_: %i\n", growSize_);
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
    uint8_t EnsureCapacityForOneMore()
    {
        uint8_t retVal = 1;
        
        // check to see if growing again is an int overflow
        if (size_ + 1 > capacity_ &&
            ((uint8_t)(capacity_ + growSize_) < capacity_))
        {
            retVal = 0;
        }
        else if (size_ + 1 > capacity_)
        {
            uint8_t  capacityNew = capacity_ + growSize_;
            T       *tableNew    = NULL;
            
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
            {
                tableNew = (T *)realloc(table_, sizeof(T) * capacityNew);
            }
            
            if (tableNew)
            {
                // Update table and capacity
                table_    = tableNew;
                capacity_ = capacityNew;
                
                // Shift all elements such that the beginning i
                
                // Check if allocation put new (empty) storage in the range
                // of contiguous data between front and back.
                // This happens when the data wraps around.
                
                if (idxBack_ <= idxFront_)
                {
                    // shift any data which is now separated by a gap.
                    
                    // Calc the amount of data wrapped around:
                    // - Subtract 0 from idxBack_ (aka the value idxBack_)
                    // - Since idxBack_ points at an empty slot, we discount 1.
                    // - Since idxBack_ could have been zero in the first place,
                    //   if it was, we check that 
                    
                    // Easier to check for special case of idxBack_ == 0
                    if (idxBack_ == 0)
                    {
                        // No elements need to be shifted.
                        // Simply adjusting idxBack_ will be required.
                        // This is handled below.
                    }
                    else
                    {
                        // need to shift elements.
                        // two sets:
                        // - those which were wrapped and now don't need to be
                        // - those which were wrapped, still have to be, but
                        //   at a lower index.

                        // Total number of wrapped elements:
                        uint8_t wrappedElements = idxBack_;

                        // Set 1
                        uint8_t countUnwrapSet1 = 0;
                        
                        if (growSize_ >= wrappedElements)
                        {
                            countUnwrapSet1 = wrappedElements;
                        }
                        else // (growSize_ < wrappedElements)
                        {
                            countUnwrapSet1 = wrappedElements - growSize_;
                        }
                        
                        // Shift the first set
                        for (uint8_t i = 0; i < countUnwrapSet1; ++i)
                        {
                            table_[size_ + i] = table_[i];
                        }
                        
                        
                        // Set 2
                        if (countUnwrapSet1 == wrappedElements)
                        {
                            // nothing to do, they all moved already
                        }
                        else
                        {
                            // Now move the second set
                            uint8_t countShiftSet2 =
                                wrappedElements - countUnwrapSet1;
                         
                            for (uint8_t i = 0; i < countShiftSet2; ++i)
                            {
                                table_[i] = table_[i + countUnwrapSet1];
                            }
                        }
                    }
                    
                    // Set idxBack_ to new location
                    // check for wrap by forcing uint8_t math, which would
                    // otherwise get missed if it lived in the 'if' condition
                    uint8_t idxBackNew = idxBack_ - growSize_;
                    if (idxBackNew > idxBack_)
                    {
                        idxBack_ = idxFront_ + size_;
                    }
                    else
                    {
                        idxBack_ = idxBackNew;
                    }
                }
                else
                {
                    // new memory was added outside of the contiguous range
                    // of elements.  relative positions of idxFront_ and 
                    // idxBack_ don't need to change.
                }
            }
            else
            {
                // memory allocation failed.
                // elements still where they were, but no capacity to fit
                // new elements.
                
                retVal = 0;
            }
        }
        
        return retVal;
    }
 
    uint8_t capacity_;
    uint8_t growSize_;
    uint8_t idxFront_;
    uint8_t idxBack_;
    uint8_t size_;
 
    T *table_;
};



template<typename T>
class Queue
: protected RingBuffer<T>
{
public:
    Queue(uint8_t capacity = RingBuffer<T>::RING_BUFFER_DEFAULT_CAPACITY,
          uint8_t growSize = RingBuffer<T>::RING_BUFFER_DEFAULT_GROW_SIZE)
    : RingBuffer<T>(capacity, growSize)
    {
        // nothing to do
    }
    
    // virtual so SortedQueue can implement same function
    virtual uint8_t Push(T element)
    {
        return RingBuffer<T>::PushBack(element);
    }
    
    uint8_t Pop(T &element)
    {
        return RingBuffer<T>::PopFront(element);
    }
    
    uint8_t Peek(T &element)
    {
        return RingBuffer<T>::PeekFront(element);
    }
    
    uint8_t Size()
    {
        return RingBuffer<T>::Size();
    }
    
    uint8_t HasElement(T element)
    {
        uint8_t retVal      = 0;
        uint8_t tmpRetParam;
        
        if (RingBuffer<T>::FindIdxFirst(element, tmpRetParam))
        {
            retVal = 1;
        }
        
        return retVal;
    }
    
    uint8_t Remove(T element)
    {
        return RingBuffer<T>::Remove(element);
    }
    
    T &operator[](uint8_t idxLogical)
    {
        return RingBuffer<T>::operator[](idxLogical);
    }
};


template<typename T, typename CMP>
class SortedQueue
: public Queue<T>
{
public:
    SortedQueue(uint8_t capacity = RingBuffer<T>::RING_BUFFER_DEFAULT_CAPACITY,
                uint8_t growSize = RingBuffer<T>::RING_BUFFER_DEFAULT_GROW_SIZE)
    : Queue<T>(capacity, growSize)
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
        if (RingBuffer<T>::PushFront(element))
        {
            retVal = 1;
            
            // Instantiate comparator
            CMP cmp;
            
            // Only try to sort if there are more than one elements
            uint8_t size = RingBuffer<T>::Size();
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











































