// synchbitmap.h 
// 
// synchronized bitmap operations


#ifndef SYNCHBITMAP_H
#define SYNCHBITMAP_H

#include "copyright.h"

class Bitmap;
class FLock;

// The following class defines a synchronized "bitmap" -- an array of bits,
// each of which can be independently set, cleared, and tested.
//

class SynchBitmap {
  public:
    SynchBitmap(int numItems);	// Initialize a SynchBitmap,
                                //with "numItems" bits
				// initially, all bits are cleared.
    ~SynchBitmap();		// De-allocate SynchBitmap
    
    void Mark(int which);   	// Set the "nth" bit
    void Clear(int which);  	// Clear the "nth" bit
    int FindAndSet();           // Return the # of a clear bit, and as a side
				// effect, set the bit. 
				// If no bits are clear, return -1.
 private:
    Bitmap  *map;
    FLock    *lock;
};

#endif // SYNCHBITMAP_H
