#ifndef __NEW_IN_PLACE_H__
#define __NEW_IN_PLACE_H__

#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>



// Placement new
//extern void *operator new(size_t, void *buf);
//void *operator new(size_t, void *buf) { return buf; }
//void *operator new (std::size_t, void * p) throw() { return p ; }
//void *operator new (size_t s, void * p) throw() { return p ; }

void* operator new(size_t size, void* const buf);


#endif  // __NEW_IN_PLACE_H__