// synchlist.h 
//	Data structures for synchronized access to a list.
//
//	Identical interface to List, except accesses are synchronized.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "list.h"
#include "synch.h"

#ifndef SYNCHLIST_H
#define SYNCHLIST_H



// The following class defines a "synchronized list" -- a list for which
// these constraints hold:
//	1. Threads trying to remove an item from a list will
//	wait until the list has an element on it.
//	2. One thread at a time can access list data structures

/**** delete
 * Liang DePeng - */
/*
template <class T>
class SynchList {
  public:
    SynchList();		// initialize a synchronized list
    ~SynchList();		// de-allocate a synchronized list

    void Append(T item);	// append item to the end of the list,
				// and wake up any thread waiting in remove

    T RemoveFront();		// remove the first item from the front of
				// the list, waiting if the list is empty

    void Apply(void (*f)(T)); // apply function to all elements in list

    void SelfTest(T value);	// test the SynchList implementation
    
  private:
    List<T> *list;		// the list of things
    Lock *lock;			// enforce mutual exclusive access to the list
    Condition *listEmpty;	// wait in Remove if the list is empty
    
    // these are only to assist SelfTest()
    SynchList<T> *selfTestPing;
    static void SelfTestHelper(void* data);
};*/
/* Liang DePeng -
 ****/

/**** modify
 * Liang DePeng + */
class __declspec(dllexport) SynchList {
public:
	SynchList(){};		// initialize a synchronized list
	virtual ~SynchList() = 0{};		// de-allocate a synchronized list
	
    virtual void Append(Item* item) = 0;	// append item to the end of the list,
											// and wake up any thread waiting in remove

    virtual Item* RemoveFront() = 0;		// remove the first item from the front of
											// the list, waiting if the list is empty

    virtual void Apply(void (*f)(Item*)) = 0;			// apply function to all elements in list

    virtual void SelfTest(Item* value) = 0;				// test the SynchList implementation
    
protected:
    List* list;			// the list of things
    FLock* lock;			// enforce mutual exclusive access to the list
    Condition *listEmpty;	// wait in Remove if the list is empty
    

};
/* Liang DePeng +
 ****/

/**** add
 * Liang DePeng + */
//宏定义函数指针
typedef SynchList* (*GetSynchList)(int (*comp)(Item* x, Item* y));  
typedef void (*DeleteSynchList)(SynchList*);

extern "C" {
	__declspec(dllexport) SynchList* GetSynchListF(int (*comp)(Item* x, Item* y));
	__declspec(dllexport) void DeleteSynchListF(SynchList*);
}

extern GetSynchList getSynchList;
extern DeleteSynchList deleteSynchList;	
/* Liang DePeng +
 ****/


#endif // SYNCHLIST_H
