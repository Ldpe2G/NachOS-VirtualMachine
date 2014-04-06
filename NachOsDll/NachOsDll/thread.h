// thread.h 
//	Data structures for managing threads.  A thread represents
//	sequential execution of code within a program.
//	So the state of a thread includes the program counter,
//	the processor registers, and the execution stack.
//	
// 	Note that because we allocate a fixed size stack for each
//	thread, it is possible to overflow the stack -- for instance,
//	by recursing to too deep a level.  The most common reason
//	for this occuring is allocating large data structures
//	on the stack.  For instance, this will cause problems:
//
//		void foo() { int buf[1000]; ...}
//
//	Instead, you should allocate all data structures dynamically:
//
//		void foo() { int *buf = new int[1000]; ...}
//
//
// 	Bad things happen if you overflow the stack, and in the worst 
//	case, the problem may not be caught explicitly.  Instead,
//	the only symptom may be bizarre segmentation faults.  (Of course,
//	other problems can cause seg faults, so that isn't a sure sign
//	that your thread stacks are too small.)
//	
//	One thing to try if you find yourself with seg faults is to
//	increase the size of thread stack -- ThreadStackSize.
//
//  	In this interface, forking a thread takes two steps.
//	We must first allocate a data structure for it: "t = new Thread".
//	Only then can we do the fork: "t->fork(f, arg)".
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.


#include "copyright.h"
#include "utility.h"
#include "sysdep.h"
#include "list.h"
//#include "machine.h"	/* Liang DePeng - */

#include "addrspace.h"

#ifndef THREAD_H
#define THREAD_H


// CPU register state to be saved on context switch.  
// The x86 needs to save only a few registers, 
// SPARC and MIPS needs to save 10 registers, 
// the Snake needs 18,
// and the RS6000 needs to save 75 (!)
// For simplicity, I just take the maximum over all architectures.

#define MachineStateSize 75 

// Max length of a thread name.
// Thread names are used for debugging purposes only
#define MAX_THREAD_NAME_LEN  30		/*zhong_expan + */

// Size of the thread's private execution stack.
// WATCH OUT IF THIS ISN'T BIG ENOUGH!!!!!
const int StackSize = (8 * 1024);	// in words


// Thread state
enum ThreadStatus { JUST_CREATED, RUNNING, READY, BLOCKED };


// The following class defines a "thread control block" -- which
// represents a single thread of execution.
//
//  Every thread has:
//     an execution stack for activation records ("stackTop" and "stack")
//     space to save CPU registers while not running ("machineState")
//     a "status" (running/ready/blocked)
//    
//  Some threads also belong to a user address space; threads
//  that only run in the kernel have a NULL address space.

/**** delete
 * Liang DePeng - */
/*class Thread: public Item{
  private:
    // NOTE: DO NOT CHANGE the order of these first two members.
    // THEY MUST be in this position for SWITCH to work.
    int *stackTop;			 // the current stack pointer
    void *machineState[MachineStateSize];  // all registers except for stackTop

  public:
    Thread(char* debugName);		// initialize a Thread 
    ~Thread(); 				// deallocate a Thread
					// NOTE -- thread being deleted
					// must not be running when delete 
					// is called

    // basic thread operations

    void Fork(VoidFunctionPtr func, void *arg); 
    				// Make thread run (*func)(arg)
    void yield();  		// Relinquish the CPU if any 
				// other thread is runnable
    void Sleep(bool finishing); // Put the thread to sleep and 
				// relinquish the processor
    void Begin();		// Startup code for the thread	
    void Finish();  		// The thread is done executing
    
    void CheckOverflow();   	// Check if thread stack has overflowed
    void setStatus(ThreadStatus st) { status = st; }
    char* getName() { return (name); }
    void Print() { std::cout << name; }
    void SelfTest();		// test whether thread impl is working

  private:
    // some of the private data for this class is listed above
    
    int *stack; 	 	// Bottom of the stack 
				// NULL if this is the main thread
				// (If NULL, don't deallocate stack)
    ThreadStatus status;	// ready, running or blocked
    char* name;

    void StackAllocate(VoidFunctionPtr func, void *arg);
    				// Allocate a stack for thread.
				// Used internally by Fork()

// A thread running a user program actually has *two* sets of CPU registers -- 
// one for its state while executing user code, one for its state 
// while executing kernel code.
	
    int userRegisters[TotalRegisters];	// user-level CPU register state

  public:
    void SaveUserState();		// save user-level register state
    void RestoreUserState();		// restore user-level register state

    AddrSpace *space;			// User code this thread is running.
};*/
/* Liang DePeng -
 ****/


/**** add
 * Liang DePeng + */
const int TotalRegisters = 40;
class  __declspec(dllexport) Thread: public Item{
public:
    // NOTE: DO NOT CHANGE the order of these first two members.
    // THEY MUST be in this position for SWITCH to work.
    int *stackTop;			 // the current stack pointer
    void *machineState[MachineStateSize];  // all registers except for stackTop
	
public:
    Thread(char* debugName);		// initialize a Thread 
	virtual ~Thread() = 0{}; 				// deallocate a Thread
					// NOTE -- thread being deleted
					// must not be running when delete 
					// is called

    // basic thread operations

    virtual void Fork(VoidFunctionPtr func, void *arg) = 0; 
    				// Make thread run (*func)(arg)
    virtual void yield() = 0;  		// Relinquish the CPU if any 
									// other thread is runnable
    virtual void Ssleep(bool finishing) = 0; // Put the thread to sleep and 
											// relinquish the processor
    virtual void Begin() = 0;			// Startup code for the thread	
    virtual void Finish() = 0;  		// The thread is done executing
    
    virtual void CheckOverflow() = 0;   	// Check if thread stack has overflowed
    virtual void setStatus(ThreadStatus st) = 0;// { status = st; }
    virtual char* getName() = 0;	// { return (name); }
    virtual void Print() = 0;		//{ std::cout << name; }
    virtual void SelfTest() = 0;		// test whether thread impl is working

protected:
    // some of the private data for this class is listed above
    
    int *stack; 	 	// Bottom of the stack 
				// NULL if this is the main thread
				// (If NULL, don't deallocate stack)
    ThreadStatus status;	// ready, running or blocked
    char* name;

    void StackAllocate(VoidFunctionPtr func, void *arg);
    				// Allocate a stack for thread.
				// Used internally by Fork()

// A thread running a user program actually has *two* sets of CPU registers -- 
// one for its state while executing user code, one for its state 
// while executing kernel code.
	
    int userRegisters[TotalRegisters];	// user-level CPU register state

public:
    virtual void SaveUserState() = 0;			// save user-level register state
    virtual void RestoreUserState() = 0;		// restore user-level register state

    AddrSpace *space;							// User code this thread is running.
};

//宏定义函数指针
typedef Thread* (*GetThread)(char* debugName);  
typedef void (*DeleteThread)(Thread*);

extern "C" {
	__declspec(dllexport) Thread* GetThreadF(char* debugName);
	__declspec(dllexport) void DeleteThreadF(Thread*);
}

extern GetThread getThread;
extern DeleteThread deleteThread;	
/* Liang DePeng +
 ****/

// external function, dummy routine whose sole job is to call Thread::Print
extern "C" void ThreadPrint(Item *t);	 

// Magical machine-dependent routines, defined in switch.s

extern "C" {
	// First frame on thread execution stack; 
	//   	call ThreadBegin
	//	call "func"
	//	(when func returns, if ever) call ThreadFinish()
	void ThreadRoot();

	// Stop running oldThread and start running newThread
	void SWITCH(Thread *oldThread, Thread *newThread);
}

#endif // THREAD_H
