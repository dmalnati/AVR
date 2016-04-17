#ifndef __NEW_H__
#define __NEW_H__


inline void* operator new(size_t, void* const buf) { return buf; }


#endif  // __NEW_H__