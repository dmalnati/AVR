#ifndef __NEW_IN_PLACE_H__
#define __NEW_IN_PLACE_H__

// Placement new
void *operator new(size_t, void *buf) { return buf; }

#endif  // __NEW_IN_PLACE_H__