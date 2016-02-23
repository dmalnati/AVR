#ifndef __IVM_H__
#define __IVM_H__


#include <util/atomic.h>

#include "PCIntEventHandler.h"


class PCIntEventHandler;

class Ivm
{
    friend class PCIntEventHandler;
    
public:
    static Ivm &GetInstance()
    {
        static Ivm ivm;
        
        return ivm;
    }
    
    ~Ivm() { }

    //////////////////////////////////////////////////////////////////////
    //
    // These functions are only called from "Main Thread" code.
    //
    // They require protection from reentrant effects since they access
    // and lead to data structures which are also accessible from ISRs.
    //
    //////////////////////////////////////////////////////////////////////

    uint8_t RegisterPCIntEventHandler(PCIntEventHandler *pcieh);
    uint8_t DeRegisterPCIntEventHandler(PCIntEventHandler *pcieh);
    
    
    
    
    

    //
    // This one is directly called by ISRs, and routes to OnPortPinStateChange
    // as found by ::GetInstance()
    //
    // Should be private but how do you make an actual ISR a friend?
    //
    static void
    OnISR(uint8_t port,
          uint8_t bitmapPortPinState,
          uint8_t bitmapPortPinStateLast);
    
private:

    // Cannot instantiate directly -- singleton
    Ivm() { }
    
    
    
    
    // Functions which bubble up actual hardware ISRs
    void
    OnPortPinStateChange(uint8_t port,
                         uint8_t portPin,
                         uint8_t changeDir);

                                  
                                  
                                  
                                  
    // Architecture-specific implementations for these interfaces are required.
                                  
    uint8_t
    AttachInterruptForPhysicalPin(uint8_t            physicalPin,
                                  PCIntEventHandler *pcieh);

    uint8_t
    DetachInterruptForPhysicalPin(uint8_t physicalPin);

    PCIntEventHandler *
    GetPCIntEventHandlerByPortAndPortPin(uint8_t port, uint8_t portPin);
    
    uint8_t
    InterruptIsActiveForPhysicalPin(uint8_t physicalPin);

    uint8_t
    GetPortAndPortPinFromPhysicalPin(uint8_t  physicalPin,
                                     uint8_t *port,
                                     uint8_t *portPin);

};



 
 #endif // __IVM_H__
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 