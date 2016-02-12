#include <Arduino.h>

#include "ISR.h"
#include "Evm.h"


/*
 * Main Principle of Architecture:
 * - ISR-invoked code can only add to Evm InterruptEventHandler list
 *   - and never enables interrupts while running
 *
 * - Evm-invoked code can read/write the InterruptEventHandler list
 *   - but does so only in protected code which prevents ISRs from firing
 *     - and takes care to not rely on the structure being unchanged outside
 *       of protected areas.
 *
 * - This means most complexity still lies in main thread code, ISRs simply
 *   queue data for Evm to deal with later.
 *
 * - Additionally this means there can only be 1 of 2 types of code running
 *   while accessing data which can be reached from both ISR and Evm.
 *
 * - Lastly this makes ISR-invoked code return very quickly, maximizing
 *   opportunity for other ISRs to be serviced, since they are blocked out
 *   while other ISRs are running.
 *
 */

 
// A simple typedef for function prototype of ISR handling functions
typedef void (*ISRFn)(void);


// Prototype of a function defined later in the file
static ISRFn ISR_GetISRFnFromPin(uint8_t pin);


// Storage for mapping between pin and InterruptEventHandler
static InterruptEventHandler *pin__ieh[24] = { 0 };



//////////////////////////////////////////////////////////////////////
//
// Externally visible code
//
//////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////
//
// These functions are only called from "Main Thread" code.
//
// They require protection from reentrant effects since they access
// and lead to data structures which are also accessible from ISRs.
//
//////////////////////////////////////////////////////////////////////

void ISR_RegisterForInterruptEvent(InterruptEventHandler *ieh)
{
    noInterrupts();
    
    // find the ISR corresponding to the pin number, if supported
    ISRFn fn = ISR_GetISRFnFromPin(ieh->GetPin());
    
    if (fn)
    {
        // prevent overwriting -- only save the object if slot empty
        if (!pin__ieh[ieh->GetPin()])
        {
            // Record object related to pin number
            pin__ieh[ieh->GetPin()] = ieh;
            
            // Use Arduino libs to attach to interrupt
            attachInterrupt(digitalPinToInterrupt(ieh->GetPin()),
                            fn,
                            ieh->GetMode());
                            
            attachInterrupt(PCINT21,
                            fn,
                            ieh->GetMode());
        }
    }
    
    interrupts();
}

void ISR_DeRegisterForInterruptEvent(InterruptEventHandler *ieh)
{
    noInterrupts();
    
    // Find the ISR corresponding to the pin number
    // This is only to validate that the pin number is supported
    ISRFn fn = ISR_GetISRFnFromPin(ieh->GetPin());
    
    if (fn)
    {
        // Use Arduino libs to detach from interrupt
        detachInterrupt(digitalPinToInterrupt(ieh->GetPin()));

        // Remove object from table
        pin__ieh[ieh->GetPin()] = NULL;
        
        // dequeue any timed events already scheduled
        Evm::GetInstance().DeRegisterInterruptEventHandler(ieh);
    }
    
    interrupts();
}







//////////////////////////////////////////////////////////////////////
//
// Internal code
//
//////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////
//
// This function handles interfacing with ISRs
//
// This function is only called from an ISR.
//
// As a result, no protection from reentrant effects is required.
//
//////////////////////////////////////////////////////////////////////

void ISR_RouteInterruptOnPinToEventHandler(uint8_t pin)
{
    // Find the ISR corresponding to the pin number
    // This is only to validate that the pin number is supported
    ISRFn fn = ISR_GetISRFnFromPin(pin);
    
    if (fn)
    {
        InterruptEventHandler *ieh = pin__ieh[pin];
        
        // make sure there is an object stored here
        if (ieh)
        {
            digitalWrite(5, HIGH);
            Evm::GetInstance().RegisterInterruptEventHandler(ieh);
            digitalWrite(5, LOW);
        }
    }
}
    


//////////////////////////////////////////////////////////////////////
//
// The actual literal functions mapping to/from pin interrupts
//
// These functions are only called from an ISR.
//
// As a result, no protection from reentrant effects is required.
//
//////////////////////////////////////////////////////////////////////

static void ISR_FOR_PIN_0()  { ISR_RouteInterruptOnPinToEventHandler(0);  }
static void ISR_FOR_PIN_1()  { ISR_RouteInterruptOnPinToEventHandler(1);  }
static void ISR_FOR_PIN_2()  { ISR_RouteInterruptOnPinToEventHandler(2);  }
static void ISR_FOR_PIN_3()  { ISR_RouteInterruptOnPinToEventHandler(3);  }
static void ISR_FOR_PIN_4()  { ISR_RouteInterruptOnPinToEventHandler(4);  }
static void ISR_FOR_PIN_5()  { ISR_RouteInterruptOnPinToEventHandler(5);  }
static void ISR_FOR_PIN_6()  { ISR_RouteInterruptOnPinToEventHandler(6);  }
static void ISR_FOR_PIN_7()  { ISR_RouteInterruptOnPinToEventHandler(7);  }
static void ISR_FOR_PIN_8()  { ISR_RouteInterruptOnPinToEventHandler(8);  }
static void ISR_FOR_PIN_9()  { ISR_RouteInterruptOnPinToEventHandler(9);  }
static void ISR_FOR_PIN_10() { ISR_RouteInterruptOnPinToEventHandler(10); }
static void ISR_FOR_PIN_11() { ISR_RouteInterruptOnPinToEventHandler(11); }
static void ISR_FOR_PIN_12() { ISR_RouteInterruptOnPinToEventHandler(12); }
static void ISR_FOR_PIN_13() { ISR_RouteInterruptOnPinToEventHandler(13); }
static void ISR_FOR_PIN_14() { ISR_RouteInterruptOnPinToEventHandler(14); }
static void ISR_FOR_PIN_15() { ISR_RouteInterruptOnPinToEventHandler(15); }
static void ISR_FOR_PIN_16() { ISR_RouteInterruptOnPinToEventHandler(16); }
static void ISR_FOR_PIN_17() { ISR_RouteInterruptOnPinToEventHandler(17); }
static void ISR_FOR_PIN_18() { ISR_RouteInterruptOnPinToEventHandler(18); }
static void ISR_FOR_PIN_19() { ISR_RouteInterruptOnPinToEventHandler(19); }
static void ISR_FOR_PIN_20() { ISR_RouteInterruptOnPinToEventHandler(20); }
static void ISR_FOR_PIN_21() { ISR_RouteInterruptOnPinToEventHandler(21); }
static void ISR_FOR_PIN_22() { ISR_RouteInterruptOnPinToEventHandler(22); }
static void ISR_FOR_PIN_23() { ISR_RouteInterruptOnPinToEventHandler(23); }


//////////////////////////////////////////////////////////////////////
//
// This function is called from both ISRs as well as "Main Thread" code.
//
// However, in both cases, it is already protected from reentrant effects
// earlier in the stack, so no additional protection is required here.
//
//////////////////////////////////////////////////////////////////////
static ISRFn ISR_GetISRFnFromPin(uint8_t pin)
{
    ISRFn fn = NULL;
    
    switch (pin)
    {
        case 0:  fn = ISR_FOR_PIN_0;  break;
        case 1:  fn = ISR_FOR_PIN_1;  break;
        case 2:  fn = ISR_FOR_PIN_2;  break;
        case 3:  fn = ISR_FOR_PIN_3;  break;
        case 4:  fn = ISR_FOR_PIN_4;  break;
        case 5:  fn = ISR_FOR_PIN_5;  break;
        case 6:  fn = ISR_FOR_PIN_6;  break;
        case 7:  fn = ISR_FOR_PIN_7;  break;
        case 8:  fn = ISR_FOR_PIN_8;  break;
        case 9:  fn = ISR_FOR_PIN_9;  break;
        case 10: fn = ISR_FOR_PIN_10; break;
        case 11: fn = ISR_FOR_PIN_11; break;
        case 12: fn = ISR_FOR_PIN_12; break;
        case 13: fn = ISR_FOR_PIN_13; break;
        case 14: fn = ISR_FOR_PIN_14; break;
        case 15: fn = ISR_FOR_PIN_15; break;
        case 16: fn = ISR_FOR_PIN_16; break;
        case 17: fn = ISR_FOR_PIN_17; break;
        case 18: fn = ISR_FOR_PIN_18; break;
        case 19: fn = ISR_FOR_PIN_19; break;
        case 20: fn = ISR_FOR_PIN_20; break;
        case 21: fn = ISR_FOR_PIN_21; break;
        case 22: fn = ISR_FOR_PIN_22; break;
        case 23: fn = ISR_FOR_PIN_23; break;
        
        default: break;
    }
    
    return fn;
}


















