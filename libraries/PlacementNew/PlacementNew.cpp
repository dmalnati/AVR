#include <Arduino.h>
#include "PlacementNew.h"

//void *operator new(size_t, void *buf) { return buf; }

void* operator new(size_t size, void* const buf) { return buf; }