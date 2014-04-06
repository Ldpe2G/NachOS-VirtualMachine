// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables.
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Once we'e implemented one set of higher level atomic operations,
// we can implement others using that implementation.  We illustrate
// this by implementing locks and condition variables on top of 
// semaphores, instead of directly enabling and disabling interrupts.
//
// Locks are implemented using a semaphore to keep track of
// whether the lock is held or not -- a semaphore value of 0 means
// the lock is busy; a semaphore value of 1 means the lock is free.
//
// The implementation of condition variables using semaphores is
// a bit trickier, as explained below under Condition::Wait.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "kernel.h"
//#include "main.h"


int SyfakeCompare (Item *x, Item *y){	
	return 0;
}

FSemaphore::FSemaphore(char* debugName, int initialValue){
	name = debugName;
	value = initialValue;
	ThreadQueue = getSortedList(SyfakeCompare);	
};	


/**** add
 * Liang DePeng + */
class __declspec(dllexport) MySemaphore: public FSemaphore{
public:
	MySemaphore(char* debugName, int initialValue):FSemaphore(debugName, initialValue){}

	~MySemaphore();   // de-allocate semaphore
	char* getName(){ return name;}	// debugging assist
    
    void P();			// these are the only operations on a semaphore
    void V();			// they are both *atomic*
    void SelfTest();	// test routine for semaphore implementation
};
/* Liang DePeng +
 ****/

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

/**** delete
 * Liang DePeng - */
/*
static int fakeCompare (Item *x, Item *y){	
	return 0;
}

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    ThreadQueue = (* getList)(fakeCompare);
}*/
/* Liang DePeng -
 ****/


//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

/**** delete
 * Liang DePeng - */
/*
Semaphore::~Semaphore()
{
    (* deleteList)(ThreadQueue);	
}*/
/* Liang DePeng -
 ****/

/**** add
 * Liang DePeng + */
MySemaphore::~MySemaphore(){
   deleteSortedList(ThreadQueue);	
}
/* Liang DePeng +
 ****/

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

/**** delete
 * Liang DePeng - */
/*
void Semaphore::P()
{
    //Interrupt *interrupt = kernel->interrupt;		Liang DePeng - 
    Thread *currentThread = kernel->currentThread;
    
    // disable interrupts
    IntStatus oldLevel = (* kernel->setLevel)(IntOff);	 Liang DePeng + 
	//interrupt->SetLevel(IntOff);						 Liang DePeng -	
    
    while (value == 0) { 		// semaphore not available
		ThreadQueue->Append(currentThread);	// so go to sleep
		currentThread->Sleep(FALSE);
    } 
    value--; 			// semaphore available, consume its value
   
    // re-enable interrupts
    (void) (* kernel->setLevel)(oldLevel);	Liang DePeng +
	//interrupt->SetLevel(oldLevel);		Liang DePeng - 
}*/
/* Liang DePeng -
 ****/

/**** add
 * Liang DePeng + */
void MySemaphore::P(){

    Thread *currentThread = kernel->currentThread;
    // disable interrupts
    int oldLevel = (* kernel->setLevel)(IntOff);	 					
    
    while (value == 0) { 		// semaphore not available
		ThreadQueue->Append(currentThread);	// so go to sleep
		currentThread->Ssleep(FALSE);
    } 
    value--; 			// semaphore available, consume its value
    // re-enable interrupts
    (void) (* kernel->setLevel)(oldLevel);	
}
/* Liang DePeng +
 ****/


//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that interrupts
//	are disabled when it is called.
//----------------------------------------------------------------------

/**** delete
 * Liang DePeng - */
/*
void Semaphore::V(){
    //Interrupt *interrupt = kernel->interrupt;	Liang DePeng - 
    
    // disable interrupts
    //IntStatus oldLevel = interrupt->SetLevel(IntOff);		Liang DePeng - 
	IntStatus oldLevel = (* kernel->setLevel)(IntOff);		Liang DePeng + 

    if (!ThreadQueue->IsEmpty()) {  // make thread ready.
		kernel->scheduler->ReadyToRun((Thread* )ThreadQueue->RemoveFront());
    }
    value++;
    
    // re-enable interrupts
    //(void) interrupt->SetLevel(oldLevel);	Liang DePeng - 
	(void)(* kernel->setLevel)(oldLevel);	Liang DePeng + 
}*/
/* Liang DePeng -
 ****/

/**** add
 * Liang DePeng + */
void MySemaphore::V(){
    
    // disable interrupts
	int oldLevel = (* kernel->setLevel)(IntOff);		

    if (!ThreadQueue->IsEmpty()) {  // make thread ready.
		kernel->scheduler->ReadyToRun((Thread* )ThreadQueue->RemoveFront());
    }
    value++;
	(void)(* kernel->setLevel)(oldLevel);	 
}
/* Liang DePeng +
 ****/

//----------------------------------------------------------------------
// Semaphore::SelfTest, SelfTestHelper
// 	Test the semaphore implementation, by using a semaphore
//	to control two threads ping-ponging back and forth.
//----------------------------------------------------------------------


static FSemaphore *ping;
static void SelfTestHelper(FSemaphore *pong){
    for (int i = 0; i < 10; i++) {
        ping->P();
		pong->V();
    }
}

/**** delete
 * Liang DePeng - */
/*
void Semaphore::SelfTest(){
    Thread *helper = new Thread("ping");

    ASSERT(value == 0);		// otherwise test won't work!
    ping = new Semaphore("ping", 0);
    helper->Fork((VoidFunctionPtr) SelfTestHelper, this);
    for (int i = 0; i < 10; i++) {
        ping->V();
		this->P();
    }
    delete ping;
}*/

/**** add
 * Liang DePeng + */
void MySemaphore::SelfTest(){
    //Thread *helper = new Thread("ping");	/* Liang DePeng - */		
	Thread* helper = GetThreadF("ping");	/* Liang DePeng + */		
    ASSERT(value == 0);		// otherwise test won't work!
    ping = GetSemaphoreF("ping", 0);
    helper->Fork((VoidFunctionPtr) SelfTestHelper, this);
    for (int i = 0; i < 10; i++) {
        ping->V();
		this->P();
    }
    DeleteSemaphoreF(ping);
}

extern "C" {
	__declspec(dllexport) FSemaphore* GetSemaphoreF(char* debugName, int initialValue){
		return new MySemaphore(debugName, initialValue);
	}
	
	__declspec(dllexport) void DeleteSemaphoreF(FSemaphore* s){
		delete s;
	}
}
/* Liang DePeng +
 ****/


FLock::FLock(char* debugName){	// initialize lock to be FREE
	name = debugName;
	semaphore = GetSemaphoreF("lock", 1);	//new Semaphore("lock", 1);  // initially, unlocked
	lockHolder = NULL;
}  	


/**** add
 * Liang DePeng + */
class __declspec(dllexport) MyLock: public FLock{
public:
	MyLock(char* debugName):FLock(debugName){	// initialize lock to be FREE
	}	
	~MyLock();						// deallocate lock
    char* getName(){ return name; }	// debugging assist

    void Acquire(); 		// these are the only operations on a lock
    void Release(); 		// they are both *atomic*

    bool IsHeldByCurrentThread(){ 
    	return lockHolder == kernel->currentThread; 
	}
	// return true if the current thread 
	// holds this lock.
};
/* Liang DePeng +
 ****/

//----------------------------------------------------------------------
// Lock::Lock
// 	Initialize a lock, so that it can be used for synchronization.
//	Initially, unlocked.
//
//	"debugName" is an arbitrary name, useful for debugging.
//----------------------------------------------------------------------

/**** delete
 * Liang DePeng - */
/*
Lock::Lock(char* debugName)
{
    name = debugName;
    semaphore = new Semaphore("lock", 1);  // initially, unlocked
    lockHolder = NULL;
}*/
/* Liang DePeng -
 ****/

//----------------------------------------------------------------------
// Lock::~Lock
// 	Deallocate a lock
//----------------------------------------------------------------------

/**** add
 * Liang DePeng + */
MyLock::~MyLock()
{
    DeleteSemaphoreF(semaphore);
}
/* Liang DePeng +
 ****/

//----------------------------------------------------------------------
// Lock::Acquire
//	Atomically wait until the lock is free, then set it to busy.
//	Equivalent to Semaphore::P(), with the semaphore value of 0
//	equal to busy, and semaphore value of 1 equal to free.
//----------------------------------------------------------------------

//void Lock::Acquire()	/* Liang Depeng - */
void MyLock::Acquire(){	/* Liang Depeng + */
    semaphore->P();
    lockHolder = kernel->currentThread;
}

//----------------------------------------------------------------------
// Lock::Release
//	Atomically set lock to be free, waking up a thread waiting
//	for the lock, if any.
//	Equivalent to Semaphore::V(), with the semaphore value of 0
//	equal to busy, and semaphore value of 1 equal to free.
//
//	By convention, only the thread that acquired the lock
// 	may release it.
//---------------------------------------------------------------------

//void Lock::Release(){		/* Liang Depeng - */
void MyLock::Release(){		/* Liang Depeng + */
    ASSERT(IsHeldByCurrentThread());
    lockHolder = NULL;
    semaphore->V();
}


/**** add
 * Liang DePeng + */
extern "C" {
	__declspec(dllexport) FLock* GetLockF(char* debugName){
		return new MyLock(debugName);
	}

	__declspec(dllexport) void DeleteLockF(FLock* l){
		delete l;
	}
}
/* Liang DePeng +
 ****/


//----------------------------------------------------------------------
// Condition::Condition
// 	Initialize a condition variable, so that it can be 
//	used for synchronization.  Initially, no one is waiting
//	on the condition.
//
//	"debugName" is an arbitrary name, useful for debugging.
//----------------------------------------------------------------------
Condition::Condition(char* debugName){
    name = debugName;
    WaitQueue = getSortedList(SyfakeCompare);	/* Liang Depeng + */
}

//----------------------------------------------------------------------
// Condition::Condition
// 	Deallocate the data structures implementing a condition variable.
//----------------------------------------------------------------------

Condition::~Condition(){
   deleteSortedList(WaitQueue);			/* Liang Depeng + */
}

//----------------------------------------------------------------------
// Condition::Wait
// 	Atomically release monitor lock and go to sleep.
//	Our implementation uses semaphores to implement this, by
//	allocating a semaphore for each waiting thread.  The signaller
//	will V() this semaphore, so there is no chance the waiter
//	will miss the signal, even though the lock is released before
//	calling P().
//
//	Note: we assume Mesa-style semantics, which means that the
//	waiter must re-acquire the monitor lock when waking up.
//
//	"conditionLock" -- lock protecting the use of this condition
//----------------------------------------------------------------------

void Condition::Wait(FLock* conditionLock) {
     FSemaphore *waiter;
    
     ASSERT(conditionLock->IsHeldByCurrentThread());

     //waiter = new Semaphore("condition", 0);	/* Liang DePeng - */
	 waiter = GetSemaphoreF("condition", 0);	/* Liang DePeng + */

     WaitQueue->Append(waiter);
     conditionLock->Release();
     waiter->P();
     conditionLock->Acquire();
     //delete waiter;				/* Liang DePeng - */
	 DeleteSemaphoreF(waiter);	/* Liang DePeng + */
}

//----------------------------------------------------------------------
// Condition::Signal
// 	Wake up a thread waiting on this condition, if any.
//
//	Note: we assume Mesa-style semantics, which means that the
//	signaller doesn't give up control immediately to the thread
//	being woken up (unlike Hoare-style).
//
//	Also note: we assume the caller holds the monitor lock
//	(unlike what is described in Birrell's paper).  This allows
//	us to access waitQueue without disabling interrupts.
//
//	"conditionLock" -- lock protecting the use of this condition
//----------------------------------------------------------------------

void Condition::Signal(FLock* conditionLock){
    MySemaphore *waiter;	
    
    ASSERT(conditionLock->IsHeldByCurrentThread());
    
    if (!WaitQueue->IsEmpty()) {
        waiter = (MySemaphore* )WaitQueue->RemoveFront();
		waiter->V();
    }
}

//----------------------------------------------------------------------
// Condition::Broadcast
// 	Wake up all threads waiting on this condition, if any.
//
//	"conditionLock" -- lock protecting the use of this condition
//----------------------------------------------------------------------

void Condition::Broadcast(FLock* conditionLock) {
    while (!WaitQueue->IsEmpty()) {
        Signal(conditionLock);
    }
}
