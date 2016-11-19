#ifndef __IVM_H__
#define __IVM_H__


#include <util/atomic.h>

#include "PAL.h"
#include "PCIntEventHandler.h"
#include "BADISREventHandler.h"


class PCIntEventHandler;
class BADISREventHandler;

class Ivm
{
    friend class PCIntEventHandler;
    
public:
    
    ~Ivm() { }

    //////////////////////////////////////////////////////////////////////
    //
    // These functions are only called from "Main Thread" code.
    //
    // They require protection from reentrant effects since they access
    // and lead to data structures which are also accessible from ISRs.
    //
    //////////////////////////////////////////////////////////////////////

    static uint8_t RegisterPCIntEventHandler(PCIntEventHandler *pcieh);
    static uint8_t DeRegisterPCIntEventHandler(PCIntEventHandler *pcieh);


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
    
    
    
    
    
    //////////////////////////////////////////////////////////////////////
    //
    // These functions are only called from "Main Thread" code.
    //
    // They require protection from reentrant effects since they access
    // and lead to data structures which are also accessible from ISRs.
    //
    //////////////////////////////////////////////////////////////////////
    static uint8_t RegisterBADISREventHandler(BADISREventHandler *beh);
    static uint8_t DeRegisterBADISREventHandler(BADISREventHandler *beh);
    
    
    //
    // Static function for handling BADISR
    //
    // Should be private but how do you make an actual ISR a friend?
    //
    static void
    OnBADISR();
    
    
    
private:
    // Cannot instantiate directly -- singleton
    Ivm() { }
    
    
    
    
    // Functions which bubble up actual hardware ISRs
    static void
    OnPortPinStateChange(uint8_t port,
                         uint8_t portPin,
                         uint8_t changeDir);

                                  
                                  
                                  
                                  
    // Architecture-specific implementations for these interfaces are required.
                                  
    static uint8_t
    AttachInterruptForPhysicalPin(uint8_t            physicalPin,
                                  PCIntEventHandler *pcieh);

    static uint8_t
    DetachInterruptForPhysicalPin(uint8_t physicalPin);

    static PCIntEventHandler *
    GetPCIntEventHandlerByPortAndPortPin(uint8_t port, uint8_t portPin);
    
    static uint8_t
    InterruptIsActiveForPhysicalPin(uint8_t physicalPin);

    static uint8_t
    AttachBADISREventHandler(BADISREventHandler *beh);
    
    static uint8_t
    DetachBADISREventHandler();
    
    static BADISREventHandler *
    GetBADISREventHandler();
                                     
};



 
 #endif // __IVM_H__
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 