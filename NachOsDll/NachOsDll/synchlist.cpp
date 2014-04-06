// synchlist.cc
//	Routines for synchronized access to a list.
//
// 	Implemented in "monitor"-style -- surround each procedure with a
// 	lock acquire and release pair, using condition signal and wait for
// 	synchronization.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synchlist.h"


/**** add
 * Liang DePeng + */
class __declspec(dllexport) MySynchList : public SynchList {
public:
    MySynchList(int (*comp)(Item* x, Item* y));
    ~MySynchList();		// base class destructor called automatically

	void Append(Item* item);	// append item to the end of the list,
											// and wake up any thread waiting in remove

    Item* RemoveFront();					// remove the first item from the front of
											// the list, waiting if the list is empty
	void Apply(void (*f)(Item*));			// apply function to all elements in list

    void SelfTest(Item* value);		// test the SynchList implementation
protected:
	// these are only to assist SelfTest()
    SynchList* selfTestPing;
    static void SelfTestHelper(void* data);
};
/* Liang DePeng +
 ****/


//----------------------------------------------------------------------
// SynchList<T>::SynchList
//	Allocate and initialize the data structures needed for a 
//	synchronized list, empty to start with.
//	Elements can now be added to the list.
//----------------------------------------------------------------------
/**** delete
 * Liang DePeng - */
/*
template <class T>
SynchList<T>::SynchList()
{
    list = new List<T>;
    lock = new Lock("list lock"); 
    listEmpty = new Condition("list empty cond");
}*/
/* Liang DePeng -
 ****/

/**** add
 * Liang DePeng + */
MySynchList::MySynchList(int (*comp)(Item* x, Item* y)):SynchList(){
	list = getSortedList(comp);
    lock = GetLockF("list lock"); 
    listEmpty = new Condition("list empty cond");
}
/* Liang DePeng +
 ****/


//----------------------------------------------------------------------
// SynchList<T>::~SynchList
//	De-allocate the data structures created for synchronizing a list. 
//----------------------------------------------------------------------
/**** delete
 * Liang DePeng - */
/*
template <class T>
SynchList<T>::~SynchList()
{ 
    delete listEmpty;
    delete lock;
    delete list;
}*/
/* Liang DePeng -
 ****/

/**** add
 * Liang DePeng + */
MySynchList::~MySynchList(){ 
    delete listEmpty;
    DeleteLockF(lock);
    deleteSortedList(list);
}
/* Liang DePeng +
 ****/

//----------------------------------------------------------------------
// SynchList<T>::Append
//      Append an "item" to the end of the list.  Wake up anyone
//	waiting for an element to be appended.
//
//	"item" is the thing to put on the list. 
//----------------------------------------------------------------------
/**** delete
 * Liang DePeng - */
/*
template <class T>
void SynchList<T>::Append(T item)
{
    lock->Acquire();		// enforce mutual exclusive access to the list 
    list->Append(item);
    listEmpty->Signal(lock);	// wake up a waiter, if any
    lock->Release();
}*/
/* Liang DePeng -
 ****/

/**** add
 * Liang DePeng + */
void MySynchList::Append(Item* item){
    lock->Acquire();		// enforce mutual exclusive access to the list 
    list->Append(item);
    listEmpty->Signal(lock);	// wake up a waiter, if any
    lock->Release();
}
/* Liang DePeng +
 ****/

//----------------------------------------------------------------------
// SynchList<T>::RemoveFront
//      Remove an "item" from the beginning of the list.  Wait if
//	the list is empty.
// Returns:
//	The removed item. 
//----------------------------------------------------------------------
/**** delete
 * Liang DePeng - */
/*
template <class T>
T SynchList<T>::RemoveFront()
{
    T item;

    lock->Acquire();			// enforce mutual exclusion
    while (list->IsEmpty())
	listEmpty->Wait(lock);		// wait until list isn't empty
    item = list->RemoveFront();
    lock->Release();
    return item;
}*/
/* Liang DePeng -
 ****/

/**** add
 * Liang DePeng + */
Item* MySynchList::RemoveFront(){
    Item* item;

    lock->Acquire();			// enforce mutual exclusion
    while(list->IsEmpty())
		listEmpty->Wait(lock);		// wait until list isn't empty
    item = list->RemoveFront();
    lock->Release();
    return item;
}
/* Liang DePeng +
 ****/


//----------------------------------------------------------------------
// SynchList<T>::Apply
//      Apply function to every item on a list.
//
//      "func" -- the function to apply
//----------------------------------------------------------------------

/**** delete
 * Liang DePeng - */
/*
template <class T>
void SynchList<T>::Apply(void (*func)(T))
{
    lock->Acquire();			// enforce mutual exclusion
    list->Apply(func);
    lock->Release();
}*/
/* Liang DePeng -
 ****/

/**** add
 * Liang DePeng + */
void MySynchList::Apply(void (*func)(Item*)){
    lock->Acquire();			// enforce mutual exclusion
    list->Apply(func);
    lock->Release();
}
/* Liang DePeng +
 ****/

//----------------------------------------------------------------------
// SynchList<T>::SelfTest, SelfTestHelper
//	Test whether the SynchList implementation is working,
//	by having two threads ping-pong a value between them
//	using two synchronized lists.
//----------------------------------------------------------------------

/**** delete
 * Liang DePeng - */
/*
template <class T>
void SynchList<T>::SelfTestHelper (void* data) 
{
    SynchList<T>* _this = (SynchList<T>*)data;
    for (int i = 0; i < 10; i++) {
        _this->Append(_this->selfTestPing->RemoveFront());
    }
}

template <class T>
void SynchList<T>::SelfTest(T val)
{
    Thread *helper = new Thread("ping");
    
    ASSERT(list->IsEmpty());
    selfTestPing = new SynchList<T>;
    helper->Fork(SynchList<T>::SelfTestHelper, this);
    for (int i = 0; i < 10; i++) {
        selfTestPing->Append(val);
		ASSERT(val == this->RemoveFront());
    }
    delete selfTestPing;
}*/
/* Liang DePeng -
 ****/


/**** add
 * Liang DePeng + */
void MySynchList::SelfTestHelper(void* data) 
{
    MySynchList* _this = (MySynchList*)data;
    for (int i = 0; i < 10; i++) {
        _this->Append(_this->selfTestPing->RemoveFront());
    }
}


int slfakeCompare (Item *x, Item *y){	
	return 0;
}

void MySynchList::SelfTest(Item* val)
{
    //Thread *helper = new Thread("ping");
    Thread* helper = GetThreadF("ping");	

    ASSERT(list->IsEmpty());
    selfTestPing = new MySynchList(slfakeCompare);
    helper->Fork(MySynchList::SelfTestHelper, this);
    for (int i = 0; i < 10; i++) {
        selfTestPing->Append(val);
		ASSERT(val == this->RemoveFront());
    }
    delete selfTestPing;
}


extern "C" {
	__declspec(dllexport) SynchList* GetSynchListF(int (*comp)(Item* x, Item* y)){
		return new MySynchList(comp);
	}
	
	__declspec(dllexport) void DeleteSynchListF(SynchList* sl){
		delete sl;
	}
}

/* Liang DePeng +
 ****/