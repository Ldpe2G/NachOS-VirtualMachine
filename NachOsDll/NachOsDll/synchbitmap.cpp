// synchbitmap.cc 
// synchronized bitmap operations

#include "copyright.h"
#include "debug.h"
#include "synchbitmap.h"
#include "bitmap.h"
#include "synch.h"

//----------------------------------------------------------------------
// SynchBitmap::SynchBitmap
// 	Initialize a bitmap with "numItems" bits, so that every bit is clear.
//	it can be added somewhere on a list.
//
//	"numItems" is the number of bits in the bitmap.
//----------------------------------------------------------------------

SynchBitmap::SynchBitmap(int numItems) 
{ 
    ASSERT(numItems > 0);
    map = new Bitmap(numItems);
    ASSERT(map != (Bitmap*)0);
    //lock = new Lock("synchbitmapLock");
	lock = GetLockF("synchbitmapLock");	

    ASSERT(lock != (FLock*)0);
}

//----------------------------------------------------------------------
// SynchBitmap::~SynchBitmap
// 	De-allocate a bitmap.
//----------------------------------------------------------------------

SynchBitmap::~SynchBitmap()
{ 
    delete map;
    delete lock;
}

//----------------------------------------------------------------------
// Synchbitmap::Set
// 	Set the "nth" bit in a bitmap.
//
//	"which" is the number of the bit to be set.
//----------------------------------------------------------------------

void
SynchBitmap::Mark(int which) 
{ 
  lock->Acquire();
  map->Mark(which);
  lock->Release();
}
    
//----------------------------------------------------------------------
// SynchBitmap::Clear
// 	Clear the "nth" bit in a bitmap.
//
//	"which" is the number of the bit to be cleared.
//----------------------------------------------------------------------

void 
SynchBitmap::Clear(int which) 
{
  lock->Acquire();
  map->Clear(which);
  lock->Release();
}

//----------------------------------------------------------------------
// SynchBitmap::Test
// 	Return TRUE if the "nth" bit is set.
//
//	"which" is the number of the bit to be tested.
//----------------------------------------------------------------------
//
//bool 
//SynchBitmap::Test(int which) const
//{
//  bool result;
//  lock->Acquire();
//  result = map->Test(which);
//  lock->Release();
//  return(result);
//}

//----------------------------------------------------------------------
// SynchBitmap::FindAndSet
// 	Return the number of the first bit which is clear.
//	As a side effect, set the bit (mark it as in use).
//	(In other words, find and allocate a bit.)
//
//	If no bits are clear, return -1.
//----------------------------------------------------------------------

int 
SynchBitmap::FindAndSet() 
{
  int result;
  lock->Acquire();
  result=map->FindAndSet();
  lock->Release();
  return(result);
}

