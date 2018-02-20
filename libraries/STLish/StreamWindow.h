#ifndef __STREAM_WINDOW_H__
#define __STREAM_WINDOW_H__



/*

Design objectives
- constantly add characters
- old ones get pushed out as buffer fills
- users can keep/get pointer to start and use as string
  - meaning null terminator
  - meaning inefficient constantly shifting left, but who cares
- uses its own buffer or someone else's?
  - could do both, but which first?
- ultimately to be used by async serial input for getline


 */

class StreamWindow
{
    
};


#endif  // __STREAM_WINDOW_H__









