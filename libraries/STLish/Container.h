#ifndef __CONTAINER_H__
#define __CONTAINER_H__


#include <util/atomic.h>

#include <stdint.h>
#include <stdlib.h>

#include "New.h"

 
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
 

// Forward Declarations
template <typename, uint8_t>           class Queue;
template <typename, uint8_t, typename> class SortedQueue;
template <typename, uint8_t>           class ListInPlace;
 

template<typename T, uint8_t CAPACITY>
class RingBuffer
{
    template <typename, uint8_t>           friend class Queue;
    template <typename, uint8_t, typename> friend class SortedQueue;
    template <typename, uint8_t>           friend class ListInPlace;
    
private:
    static const uint8_t OVERFLOW_BOUNDARY = (uint8_t)-1;
    
public:
    RingBuffer()
    : idxFront_(0)
    , idxBack_(0)
    , size_(0)
    , capacity_(CAPACITY ? CAPACITY : 1)
    , table_((T *)data_)
    {
        // Nothing to do
    }
    
    ~RingBuffer()
    {
        // Nothing to do
    }
 
    /////////// Basic Getters / Setters ///////////
    
    inline uint8_t Capacity() const { return capacity_; }
    inline uint8_t Size() const { return size_; }
    
    
    /////////// Front Access ///////////
    
    uint8_t PushFront(T element)
    {
        uint8_t retVal = CanFitOneMore();
        
        if (retVal)
        {
            DecrFront();
            
            table_[idxFront_] = element;
            
            IncrSize();
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
     
            IncrFront();
     
            DecrSize();
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
     
            IncrBack();
     
            IncrSize();
        }
        
        return retVal;
    }
    
    uint8_t PopBack(T &element)
    {
        uint8_t retVal = 0;
        
        if (size_)
        {
            retVal = 1;
            
            DecrBack();
            
            element = table_[idxBack_];
            
            DecrSize();
        }
        
        return retVal;
    }
    
    /////////// Atomic Access ///////////
    
    // This supports the movement of data between main thread code and an ISR.
    // 
    // Idea being the pusher is trying to feed data to the ISR quickly, knowing
    // that the queue may fill before the ISR has been able to consume it all.
    // However, the ISR will consume it eventually.
    //
    // Knowing that, the main thread is ok waiting (blocking) waiting to push
    // more elements.
    //
    // Waiting to push more elements should not come at the expense of the ISR
    // operation, so as little time in an atomic block should be spent as
    // possible.
    //
    // Once there is again space in the queue, it will not increase again
    // through any other means than the main blocked thread adding more
    // (which is by completing this call).
    //
    // There is no need for Pop to be atomic, the ISR cannot be interrupted.
    //
    uint8_t PushBackAtomic(T element)
    {
        uint8_t retVal = 0;
        
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            retVal = PushBack(element);
        }
        
        if (!retVal)
        {
            // Spin lock waiting until something fits
            while (!CanFitOneMore()) { }
            
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
            {
                // This should work now
                retVal = PushBack(element);
            }
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


protected:

    inline void DecrFront()
    {
        --idxFront_;
        if (idxFront_ >= capacity_) { idxFront_ = (capacity_ - 1); }
    }
    
    inline void IncrFront()
    {
        ++idxFront_;
        if (idxFront_ >= capacity_) { idxFront_ = 0; }
    }
    
    inline void IncrBack()
    {
        ++idxBack_;
        if (idxBack_ >= capacity_) { idxBack_ = 0; }
    }
    
    inline void DecrBack()
    {
        --idxBack_;
        if (idxBack_ >= capacity_) { idxBack_ = (capacity_ - 1); }
    }
    
    inline void IncrSize()
    {
        ++size_;
    }
    
    inline void DecrSize()
    {
        --size_;
    }

    inline uint8_t CanFitOneMore()
    {
        return (size_ != capacity_);
    }
    
    inline void Clear()
    {
        idxFront_ = 0;
        idxBack_  = 0;
        size_     = 0;
    }
    
private:
    
    volatile uint8_t idxFront_;
    volatile uint8_t idxBack_;
    volatile uint8_t size_;
    const uint8_t capacity_;
 
    T       *table_;
    uint8_t  data_[(CAPACITY ? CAPACITY : 1) * sizeof(T)];
};



template<typename T, uint8_t CAPACITY>
class Queue
{
public:
    Queue()
    : rb_()
    {
        // nothing to do
    }
    
    uint8_t Push(T element)
    {
        return rb_.PushBack(element);
    }
    
    uint8_t PushAtomic(T element)
    {
        return rb_.PushBackAtomic(element);
    }
    
    uint8_t Pop(T &element)
    {
        return rb_.PopFront(element);
    }
    
    uint8_t Peek(T &element)
    {
        return rb_.PeekFront(element);
    }
    
    uint8_t Size()
    {
        return rb_.Size();
    }
    
    void Clear()
    {
        rb_.Clear();
    }
    
    uint8_t HasElement(T element)
    {
        uint8_t retVal      = 0;
        uint8_t tmpRetParam;
        
        if (rb_.FindIdxFirst(element, tmpRetParam))
        {
            retVal = 1;
        }
        
        return retVal;
    }
    
    uint8_t Remove(T element)
    {
        return rb_.Remove(element);
    }
    
    T &operator[](uint8_t idxLogical)
    {
        return rb_[idxLogical];
    }
    
    
    /////////// Debug ///////////
    
#ifdef DEBUG

    void PrintDebug(const char *msg)
    {
        rb_.PrintDebug(msg);
    }
    
#endif // DEBUG


private:
    RingBuffer<T, CAPACITY> rb_;
};


template<typename T, uint8_t CAPACITY, typename CMP>
class SortedQueue
{
public:
    SortedQueue()
    : rb_()
    {
        // nothing to do
    }
    
    // inserts before the first element greater than it.
    // only applies to this element.
    // for a sorted list, strictly use this function from start to finish.
    uint8_t Push(T element)
    {
        uint8_t retVal = 0;
        
        // First, simply insert the element at the front.
        if (rb_.PushFront(element))
        {
            retVal = 1;
            
            // Instantiate comparator
            CMP cmp;
            
            // Only try to sort if there are more than one elements
            uint8_t size = rb_.Size();
            if (size > 1)
            {
                bool keepGoing = true;
                
                for (uint8_t idxFirst = 0, idxSecond = 1;
                     idxSecond < size && keepGoing;
                     ++idxFirst, ++idxSecond)
                {
                    T valFirst  = rb_[idxFirst];
                    T valSecond = rb_[idxSecond];
                    
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
                        
                        rb_[idxFirst]  = valSecond;
                        rb_[idxSecond] = valFirst;
                    }
                }
            }
        }
        
        return retVal;
    }
    
    uint8_t Pop(T &element)
    {
        return rb_.PopFront(element);
    }
    
    uint8_t Peek(T &element)
    {
        return rb_.PeekFront(element);
    }
    
    uint8_t Size()
    {
        return rb_.Size();
    }
    
    void Clear()
    {
        rb_.Clear();
    }
    
    uint8_t HasElement(T element)
    {
        uint8_t retVal      = 0;
        uint8_t tmpRetParam;
        
        if (rb_.FindIdxFirst(element, tmpRetParam))
        {
            retVal = 1;
        }
        
        return retVal;
    }
    
    uint8_t Remove(T element)
    {
        return rb_.Remove(element);
    }
    
    T &operator[](uint8_t idxLogical)
    {
        return rb_[idxLogical];
    }
    
private:
    RingBuffer<T, CAPACITY> rb_;
};



template<typename T, uint8_t CAPACITY>
class ListInPlace
{
public:
    ListInPlace()
    : rb_()
    {
        // Nothing to do
    }
    
    ~ListInPlace()
    {
        DestructElementsAndClear();
    }
    
    uint8_t Size()
    {
        return rb_.Size();
    }
    
    uint8_t DestructElementsAndClear()
    {
        // Delete elements in reverse order of construction
        for (uint8_t i = rb_.Size(); i > 0; --i)
        {
            // Call Destructor
            ((T *)&(rb_[i - 1]))->~T();
        }
        
        rb_.Clear();
        
        return 1;
    }
    
    T &operator[](uint8_t idxLogical)
    {
        return rb_[idxLogical];
    }
    
    template <typename ...Args>
    T *PushNew(Args &&... args)
    {
        T *retVal = NULL;

        if (rb_.CanFitOneMore())
        {
            // Placement New invocation with Constructor argument passing
            new ((void *)&(rb_[rb_.Size()]))
                T(static_cast<Args>(args)...);

            rb_.IncrBack();
            rb_.IncrSize();
            
            retVal = &(rb_[rb_.Size() - 1]);
        }

        return retVal;
    }
    
private:
    RingBuffer<T, CAPACITY> rb_;
};





#endif  // __CONTAINER_H__











































