#ifndef __FUNCTION_H__
#define __FUNCTION_H__


#include <string.h>

#include "New.h"



// Derived from:
// http://stackoverflow.com/questions/14739902/is-there-a-standalone-implementation-of-stdfunction
//
// Differences include:
// - Placement new instead of dynamic memory allocation
//   - Capacity set statically but enforced with static_assert
// - Used argument forwarding
// - Renamed template params and classes to highlight how I think they work
// - Rearranged order of some class elements to my taste
// - Lots of comments as I tried to figure out how it worked
//

#define FUNCTION_STORAGE_REQUIREMENT_MAX (sizeof(void *) * 4)


/*
 * This class represents a base class that a pointer can be held to.
 *
 * The only point is to hide the real type of a templated child class
 * whose type is inconvenient to know elsewhere.
 *
 */
template<typename ReturnType, typename ...FunctorArgParamTypeList>
struct FunctorCarrierBase
{
    virtual ReturnType  operator()(FunctorArgParamTypeList &&... args) = 0;
    virtual void        CloneInto(void *buf) const                     = 0;
    virtual            ~FunctorCarrierBase()                           = default;
};

/*
 * This class actually wraps an instance of a functor and allows it to be called.
 *
 * The template allows this class to perfectly match the prototype of the functor.
 *
 * The type of this class is also inconvenient to know, and is later only known
 * by another function (a constructor) which is templated itself.
 *
 * That templated constructor instantiates this class, and stores a pointer to
 * its base, thereby allowing access to the members.
 *
 */
template<typename FunctorType, typename ReturnType, typename ...FunctorArgParamTypeList>
class FunctorCarrierConcrete
: public FunctorCarrierBase<ReturnType, FunctorArgParamTypeList...>
{
public:
    FunctorCarrierConcrete(const FunctorType &f)
    : f_(f)
    {
        // Enforce a maximum size at compile-time
        using ThisClass = FunctorCarrierConcrete<FunctorType,
                                                 ReturnType,
                                                 FunctorArgParamTypeList...>;
        static_assert(sizeof(ThisClass) <= FUNCTION_STORAGE_REQUIREMENT_MAX,
                      "Functor size too large");
    }
    
    virtual ReturnType operator()(FunctorArgParamTypeList &&... args) override
    {
        // Forward arguments
        return f_(static_cast<FunctorArgParamTypeList>(args)...);
    }
    
    virtual void CloneInto(void *buf) const override
    {
        // Placement new
        new (buf) FunctorCarrierConcrete(f_);
    }
    
    
private:

    FunctorType f_;
};










template<typename FunctorType>
struct FunctorTypeExposer
{
    typedef FunctorType type;
};


// literally a specialization,
// but possibly conveniently used simply to "pattern match" against
// the parts of a literal function pointer being used?
//
// the typedef creates ::type, which is of type function pointer
template<typename ReturnType, typename ...Args>
struct FunctorTypeExposer<ReturnType(Args...)>
{
    typedef ReturnType (*type)(Args...);
};












// Just a forward declaration?  (seems yes?  what else could it be?)
template<typename FunctorFunctionSignature>
class function;

//
// Partial Template Specialization
//
// But since calling code doesn't actually instantiate like this, what causes
// the compiler to do the right thing?
//
// Also, is it a specialization of a forward declaration?
//
// Some theories:
// - primary template is empty(?), causing this specialization to be looked to
//   for an implementation, and works "inside-out" doing some kind of pattern
//   match, where:
//   - knows the specialization is leading to the FunctorFunctionSignature
//   - knows that will be made up of template params of this specialization
//   - works it out
// - ... what else?
//
template<typename ReturnType, typename ...FunctorArgParamTypeList>
class function<ReturnType(FunctorArgParamTypeList...)>
{
    using FCBType = FunctorCarrierBase<ReturnType, FunctorArgParamTypeList...>;
    
public:
    function()
    {
        InitState();
    }
    
    // Template Deduction used on constructor to identify the type of the
    // functor.
    //
    // It is this trick here which allows for a concrete FunctorCarrier to be
    // created (template-wise), and its pointer stored.
    //
    // This erases the type.
    //
    template<typename FunctorType>
    function(const FunctorType &f)
    {
        InitState();
        
        using FCCType =
            FunctorCarrierConcrete<typename FunctorTypeExposer<FunctorType>::type,
                                   ReturnType,
                                   FunctorArgParamTypeList...>;
        
        // Placement new instantiate
        new ((void *)buf_) FCCType(f);
        
        // Note that we have a working functor now
        functorIsSet_ = 1;
    }
    
    function(const function &rhs)
    {
        InitState();
        
        if (rhs.functorIsSet_)
        {
            // Clone into buf
            FCBType *fcb = (FCBType *)&(buf_[0]);

            fcb->CloneInto((void *)buf_);
            
            // Note that we have a working functor now
            functorIsSet_ = 1;
        }
    }

    
    
    ~function()
    {
        DestructFunctor();
    }
    
    
    
    template<typename FunctorType>
    function &operator=(const FunctorType &f)
    {
        // Destruct old functor if it exists
        DestructFunctor();
        
        using FCCType =
            FunctorCarrierConcrete<typename FunctorTypeExposer<FunctorType>::type,
                                   ReturnType,
                                   FunctorArgParamTypeList...>;
        
        // Placement new instantiate
        new ((void *)buf_) FCCType(f);
        
        // Note that we have a working functor now
        functorIsSet_ = 1;
        
        return *this;
    }

    function &operator=(const function &rhs)
    {
        if ((&rhs != this ) && (rhs.functorIsSet_))
        {
            // Take action, it's another instance we want to dup
            
            // Destruct old functor if it exists
            DestructFunctor();
            
            // Clone into buf
            FCBType *fcb = (FCBType *)&(rhs.buf_[0]);

            fcb->CloneInto((void *)buf_);
            
            // Note that we have a working functor now
            functorIsSet_ = 1;
            
        }
        else if (&rhs == this )
        {
            // Don't do anything, it's yourself
        }
        else // (rhs.functorIsSet_ == 0)
        {
            // Destruct old functor if it exists, which resets state, which is
            // the state the other function is in
            DestructFunctor();
        }
        
        return *this;
    }
    
    
    ReturnType operator()(FunctorArgParamTypeList &&... args)
    {
        if (functorIsSet_)
        {
            // Forward arguments to object we have in memory
            FCBType *fcb = (FCBType *)&(buf_[0]);
        
            return fcb->operator()(static_cast<FunctorArgParamTypeList>(args)...);
        }
        else
            // We don't throw exceptions, so just return a default value for the
            // given return type.
            return ReturnType();
    }
    
    
private:

    void InitState()
    {
        memset(buf_, 0, FUNCTION_STORAGE_REQUIREMENT_MAX);
        
        functorIsSet_ = 0;
    }

    void DestructFunctor()
    {
        if (functorIsSet_)
        {
            // Destroy using base class' virtual destructor, manually
            FCBType *fcb = (FCBType *)&(buf_[0]);
            
            fcb->~FCBType();
        }
        
        InitState();
    }

    uint8_t  buf_[FUNCTION_STORAGE_REQUIREMENT_MAX];
    uint8_t  functorIsSet_;
};




#endif